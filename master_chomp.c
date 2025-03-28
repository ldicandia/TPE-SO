#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"
#define MAX_PLAYERS 9

typedef struct {
  char name[16];
  unsigned int score;
  unsigned int invalid_moves;
  unsigned int valid_moves;
  unsigned short x, y;
  pid_t pid;
  bool blocked;
} Player;

typedef struct {
  unsigned short width;
  unsigned short height;
  unsigned int num_players;
  Player players[9];
  bool game_over;
  int board[];
} GameState;

typedef struct {
  sem_t A;  // Se usa para indicarle a la vista que hay cambios por imprimir
  sem_t B;  // Se usa para indicarle al master que la vista terminó de imprimir
  sem_t C;  // Mutex para evitar inanición del master al acceder al estado
  sem_t D;  // Mutex para el estado del juego
  sem_t E;  // Mutex para la siguiente variable
  unsigned int F;  // Cantidad de jugadores leyendo el estado
} GameSync;

pid_t spawn_process(const char *path, char *width, char *height) {
  pid_t pid = fork();
  if (pid == 0) {
    execl(path, path, width, height, NULL);
    perror("execl");
    exit(EXIT_FAILURE);
  }
  return pid;
}

void initialize_board(GameState *state, unsigned int seed) {
  srand(seed);
  for (int i = 0; i < state->width * state->height; i++) {
    state->board[i] = rand() % 9 + 1;  // Random rewards 0-4
  }
}

bool all_players_blocked(GameState *state) {
  for (int i = 0; i < state->num_players; i++) {
    if (!state->players[i].blocked) {
      return false;
    }
  }
  return true;
}

bool is_valid_move(GameState *state, int player_idx, unsigned char move) {
  int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  int dir = move;  // Asumiendo que move es un índice de 0 a 7
  int new_x = state->players[player_idx].x + dx[dir];
  int new_y = state->players[player_idx].y + dy[dir];
  return (new_x >= 0 && new_x < state->width && new_y >= 0 &&
          new_y < state->height &&
          state->board[new_y * state->width + new_x] > 0);
}

bool has_valid_moves(GameState *state, int player_idx) {
  int dx[8] = {0, 1,  1,  1,
               0, -1, -1, -1};  // Direcciones: N, NE, E, SE, S, SW, W, NW
  int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

  for (int move = 0; move < 8; move++) {
    int new_x = state->players[player_idx].x + dx[move];
    int new_y = state->players[player_idx].y + dy[move];
    if (new_x >= 0 && new_x < state->width && new_y >= 0 &&
        new_y < state->height) {
      if (state->board[new_y * state->width + new_x] > 0) {  // Celda con comida
        return true;
      }
    }
  }
  return false;
}

void process_move(GameState *state, GameSync *sync, int player_idx,
                  unsigned char move) {
  sem_wait(&sync->C);
  sem_wait(&sync->D);

  if (!is_valid_move(state, player_idx, move)) {
    state->players[player_idx].invalid_moves++;
  } else {
    int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    int new_x = state->players[player_idx].x + dx[move];
    int new_y = state->players[player_idx].y + dy[move];

    state->players[player_idx].score +=
        state->board[new_y * state->width + new_x];
    state->players[player_idx].x = new_x;
    state->players[player_idx].y = new_y;
    state->players[player_idx].valid_moves++;
    state->board[new_y * state->width + new_x] =
        0 - player_idx;  // Cell consumed
  }

  sem_post(&sync->D);
  sem_post(&sync->A);
  sem_wait(&sync->B);
  sem_post(&sync->C);
}

void place_players(GameState *state) {
  for (int i = 0; i < state->num_players; i++) {
    state->players[i].x = (state->width / (state->num_players + 1)) * (i + 1);
    state->players[i].y = state->height / 2;
    state->players[i].score = 0;
    state->players[i].invalid_moves = 0;
    state->players[i].valid_moves = 0;
    state->players[i].blocked = false;
    snprintf(state->players[i].name, 16, "Player%d", i + 1);
  }
}

void *create_shared_memory(const char *name, size_t size) {
  int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  if (fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(fd, size) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }
  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

int main(int argc, char *argv[]) {
  int width = 10, height = 10, delay = 200, timeout = 10;
  unsigned int seed = time(NULL);
  char *view_path = NULL;
  char *player_paths[MAX_PLAYERS];
  int num_players = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-w") == 0) {
        if (i + 1 < argc) {
            width = atoi(argv[++i]);
            if (width < 10) {
                fprintf(stderr, "Error: minimum width must be 10.\n");
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp(argv[i], "-h") == 0) {
        if (i + 1 < argc) {
            height = atoi(argv[++i]);
            if (height < 10) {
                fprintf(stderr, "Error: minimum height must be 10.\n");
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp(argv[i], "-d") == 0) {
        if (i + 1 < argc) {
            delay = atoi(argv[++i]);
            if (delay < 0) {
                fprintf(stderr, "Error: delay must be non-negative.\n");
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp(argv[i], "-t") == 0) {
        if (i + 1 < argc) {
            timeout = atoi(argv[++i]);
            if (timeout < 0) {
                fprintf(stderr, "Error: Timeout must be non-negative.\n");
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp(argv[i], "-s") == 0) {
        if (i + 1 < argc) {
            seed = atoi(argv[++i]);
        }
    } else if (strcmp(argv[i], "-v") == 0) {
        if (i + 1 < argc) {
            view_path = argv[++i];
        }
    } else if (strcmp(argv[i], "-p") == 0) {
        while (i + 1 < argc && num_players < MAX_PLAYERS &&
               argv[i + 1][0] != '-') {
            player_paths[num_players++] = argv[++i];
        }
    } else {
        fprintf(stderr, "Warning: unknown parameter %s was ignored.\n", argv[i]);
    }
}

  if (num_players < 1 || num_players > MAX_PLAYERS) {
    fprintf(stderr, "Error: Number of players must be between 1 and %d\n",
            MAX_PLAYERS);
    exit(EXIT_FAILURE);
  }
  // Crear memoria compartida para el estado del juego
  size_t game_state_size = sizeof(GameState) + width * height * sizeof(int);
  GameState *state = create_shared_memory(SHM_GAME_STATE, game_state_size);
  GameSync *sync = create_shared_memory(SHM_GAME_SYNC, sizeof(GameSync));

  sem_init(&sync->A, 1, 0);
  sem_init(&sync->B, 1, 0);
  sem_init(&sync->C, 1, 1);
  sem_init(&sync->D, 1, 1);
  sem_init(&sync->E, 1, 1);
  sync->F = 0;

  state->width = width;
  state->height = height;
  state->num_players = num_players;
  state->game_over = false;
  initialize_board(state, seed);
  place_players(state);

  int player_pipes[MAX_PLAYERS][2];
  for (int i = 0; i < num_players; i++) {
    if (pipe(player_pipes[i]) == -1) {
      perror("pipe");
      for (int j = 0; j < i; j++) { // Cerrar pipes previos
        close(player_pipes[j][0]);
        close(player_pipes[j][1]);
      }
      exit(EXIT_FAILURE);
    }
  }

  char width_str[10], height_str[10];
  sprintf(width_str, "%d", width);
  sprintf(height_str, "%d", height);

  pid_t view_pid =
      view_path ? spawn_process(view_path, width_str, height_str) : 0;
  pid_t player_pids[MAX_PLAYERS];
  for (int i = 0; i < num_players; i++) {
    player_pids[i] = fork();
    if (player_pids[i] == 0) {
      dup2(player_pipes[i][1], STDOUT_FILENO);
      close(player_pipes[i][0]);
      execl(player_paths[i], player_paths[i], width_str, height_str, NULL);
      perror("execl");
      exit(EXIT_FAILURE);
    }
    state->players[i].pid = player_pids[i];
    close(player_pipes[i][1]);
  }

  // Main game loop
  // Agrega un arreglo para rastrear el tiempo del último movimiento válido de
  // cada jugador
  time_t *last_move_times = malloc(num_players * sizeof(time_t));
  for (int i = 0; i < num_players; i++) {
    last_move_times[i] = time(NULL);  // Inicializa con el tiempo actual
  }

  fd_set readfds;
  struct timeval tv;
  int max_fd = -1;  // Calcula el máximo FD de los pipes
  for (int i = 0; i < num_players; i++) {
    if (player_pipes[i][0] > max_fd) max_fd = player_pipes[i][0];
  }

  while (!state->game_over) {
    // Verificar timeout por jugador
    time_t current_time = time(NULL);
    int active_players = 0;
    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked) {
        if (current_time - last_move_times[i] >= timeout &&
            !has_valid_moves(state, i)) {
          state->players[i].blocked = true;
          printf("Player %s blocked due to timeout or no valid moves.\n",
                 state->players[i].name);
         // active_players--;
        } else {
          active_players++;
        }
      }
    }

    // Si no quedan jugadores activos, terminar el juego
    if (active_players == 0) {
      state->game_over = true;
      break;
    }

    // Configurar select para leer de los pipes
    FD_ZERO(&readfds);
    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked) {
        FD_SET(player_pipes[i][0], &readfds);
      }
    }

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (ready == -1) {
      perror("select");
      break;
    }

    // Procesar movimientos
    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked &&
          FD_ISSET(player_pipes[i][0], &readfds)) {
        unsigned char move;
        ssize_t bytes_read = read(player_pipes[i][0], &move, sizeof(move));
        if (bytes_read <= 0) {
          state->players[i].blocked = true;
          printf("Player %s blocked due to pipe error or EOF.\n",
                 state->players[i].name);
        } else {
          process_move(state, sync, i, move);
          int new_x = state->players[i].x;
          int new_y = state->players[i].y;
          if (new_x >= 0 && new_x < state->width && new_y >= 0 &&
              new_y < state->height &&
              state->board[new_y * state->width + new_x] > 0) {
            last_move_times[i] = time(NULL);
          }
        }
      }
    }

    usleep(delay * 1000);
  }

  free(last_move_times);

  // Cleanup
  for (int i = 0; i < num_players; i++) {
    int status;
    waitpid(player_pids[i], &status, 0);
    printf("Player %s (PID %d) exited with status %d, score: %u\n",
           state->players[i].name, player_pids[i], status,
           state->players[i].score);
  }
  if (view_pid) {
    int status;
    waitpid(view_pid, &status, 0);
    printf("View (PID %d) exited with status %d\n", view_pid, status);
  }

  sem_destroy(&sync->A);
  sem_destroy(&sync->B);
  sem_destroy(&sync->C);
  sem_destroy(&sync->D);
  sem_destroy(&sync->E);

  shm_unlink(SHM_GAME_STATE);
  shm_unlink(SHM_GAME_SYNC);

  return 0;
}