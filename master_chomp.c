#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "tads/game_logic.h"
#include "tads/shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"
#define MAX_PLAYERS 9
#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 10
#define DEFAULT_DELAY 200
#define DEFAULT_TIMEOUT 10
#define MIN_WIDTH 10
#define MIN_HEIGHT 10
#define MIN_DELAY 0
#define MIN_TIMEOUT 0
#define TIMEOUT_CHECK_INTERVAL 100000 // microseconds
#define MAX_STR_LEN 10

pid_t spawn_process(const char *path, char *width, char *height);
void parse_arguments(int argc, char *argv[], int *width, int *height, int *delay, int *timeout, unsigned int *seed, char **view_path, char *player_paths[], int *num_players);
void print_parameters(int width, int height, int delay, int timeout, unsigned int seed, char *view_path, char *player_paths[], int num_players);
void check_results(int num_players, pid_t player_pids[], GameState *state, pid_t view_pid);

int main(int argc, char *argv[]) {
  int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT, delay = DEFAULT_DELAY, timeout = DEFAULT_TIMEOUT;
  unsigned int seed = time(NULL);
  char *view_path = NULL;
  char *player_paths[MAX_PLAYERS];
  int num_players = 0;

  parse_arguments(argc, argv, &width, &height, &delay, &timeout, &seed, &view_path, player_paths, &num_players);
  print_parameters(width, height, delay, timeout, seed, view_path, player_paths, num_players);

  if (num_players < 1 || num_players > MAX_PLAYERS) {
    fprintf(stderr, "Error: Number of players must be between 1 and %d\n", MAX_PLAYERS);
    exit(EXIT_FAILURE);
  }

  size_t game_state_size = sizeof(GameState) + width * height * sizeof(int);
  GameState *state = create_shared_memory(SHM_GAME_STATE, game_state_size);
  GameSync *sync = create_shared_memory(SHM_GAME_SYNC, sizeof(GameSync));

  initialize_sync(&sync->sem_view_ready, &sync->sem_master_ready, &sync->sem_state_mutex, &sync->sem_game_mutex, &sync->sem_reader_mutex, &sync->reader_count);

  state->width = width;
  state->height = height;
  state->num_players = num_players;
  state->game_over = false;
  initialize_board(state, seed);
  place_players(state);

  // Create pipes for players
  int player_pipes[MAX_PLAYERS][2];
  for (int i = 0; i < num_players; i++) {
    if (pipe(player_pipes[i]) == -1) {
      perror("pipe");
      for (int j = 0; j < i; j++) {
        close(player_pipes[j][0]);
        close(player_pipes[j][1]);
      }
      exit(EXIT_FAILURE);
    }
  }

  char width_str[MAX_STR_LEN], height_str[MAX_STR_LEN];
  sprintf(width_str, "%d", width);
  sprintf(height_str, "%d", height);

  pid_t view_pid = view_path ? spawn_process(view_path, width_str, height_str) : 0;

  // Spawn player processes
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

  time_t *last_move_times = malloc(num_players * sizeof(time_t));
  for (int i = 0; i < num_players; i++) {
    last_move_times[i] = time(NULL);
  }

  fd_set readfds;
  struct timeval tv;
  int max_fd = -1;
  for (int i = 0; i < num_players; i++) {
    if (player_pipes[i][0] > max_fd)
      max_fd = player_pipes[i][0];
  }

  int blocked_players = 0;
  while (!state->game_over) {
    time_t current_time = time(NULL);

    // Check for player timeouts
    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked) {
        if (!has_valid_moves(state, i)) {
          state->players[i].blocked = true;
          blocked_players++;
        } else if (current_time - last_move_times[i] >= timeout) {
          state->players[i].blocked = true;
          blocked_players++;
        }
      }
    }

    if (blocked_players == num_players) {
      state->game_over = true;
      if (view_pid) {
        sem_post(&sync->sem_view_ready);
        sem_wait(&sync->sem_master_ready);
      }
    }

    // Read moves from players
    FD_ZERO(&readfds);
    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked) {
        FD_SET(player_pipes[i][0], &readfds);
      }
    }

    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT_CHECK_INTERVAL;

    int ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);
    if (ready == -1) {
      perror("select");
      break;
    }

    for (int i = 0; i < num_players; i++) {
      if (!state->players[i].blocked && FD_ISSET(player_pipes[i][0], &readfds)) {
        unsigned char move;
        ssize_t bytes_read = read(player_pipes[i][0], &move, sizeof(move));
        if (bytes_read <= 0) {
          state->players[i].blocked = true;
          blocked_players++;
        } else {
          sem_wait(&sync->sem_state_mutex);
          sem_wait(&sync->sem_game_mutex);

          process_move(state, i, move);

          sem_post(&sync->sem_game_mutex);
          if (view_pid) {
            sem_post(&sync->sem_view_ready);
            sem_wait(&sync->sem_master_ready);
          }
          sem_post(&sync->sem_state_mutex);

          last_move_times[i] = time(NULL);
        }
      }
    }

    if (view_pid) {
      usleep(delay * 1000);
    }
  }

  free(last_move_times);

  // Cleanup
  for (int i = 0; i < num_players; i++) {
    close(player_pipes[i][0]);
  }

  check_results(num_players, player_pids, state, view_pid);

  destroy_sync(&sync->sem_view_ready, &sync->sem_master_ready, &sync->sem_state_mutex, &sync->sem_game_mutex, &sync->sem_reader_mutex);
  destroy_shared_memory(SHM_GAME_STATE, state, game_state_size);
  destroy_shared_memory(SHM_GAME_SYNC, sync, sizeof(GameSync));

  return 0;
}

pid_t spawn_process(const char *path, char *width, char *height) {
  pid_t pid = fork();
  if (pid == 0) {
    execl(path, path, width, height, NULL);
    perror("execl");
    exit(EXIT_FAILURE);
  }
  return pid;
}

void check_results(int num_players, pid_t player_pids[], GameState *state, pid_t view_pid) {
  for (int i = 0; i < num_players; i++) {
    int status;
    waitpid(player_pids[i], &status, 0);
    if (WIFEXITED(status)) {
      printf("Player %d || Exit status %d || Score = %d\n", i + 1, WEXITSTATUS(status), state->players[i].score);
    } else if (WIFSIGNALED(status)) {
      printf("Player %d || Killed by signal %d\n", i + 1, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
      printf("Player %d || Stopped by signal %d\n", i + 1, WSTOPSIG(status));
    } else {
      printf("Player %d || Unknown status\n", i);
    }
  }

  if (view_pid) {
    int status;
    waitpid(view_pid, &status, 0);
    printf("View (PID %d) exited with status %d\n", view_pid, status);
  }
}

void parse_arguments(int argc, char *argv[], int *width, int *height, int *delay, int *timeout, unsigned int *seed, char **view_path, char *player_paths[], int *num_players) {
  int opt;
  while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
    switch (opt) {
    case 'w':
      *width = atoi(optarg);
      if (*width < MIN_WIDTH) {
        fprintf(stderr, "Error: minimum width must be %d.\n", MIN_WIDTH);
        exit(EXIT_FAILURE);
      }
      break;
    case 'h':
      *height = atoi(optarg);
      if (*height < MIN_HEIGHT) {
        fprintf(stderr, "Error: minimum height must be %d.\n", MIN_HEIGHT);
        exit(EXIT_FAILURE);
      }
      break;
    case 'd':
      *delay = atoi(optarg);
      if (*delay < MIN_DELAY) {
        fprintf(stderr, "Error: delay must be non-negative.\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 't':
      *timeout = atoi(optarg);
      if (*timeout < MIN_TIMEOUT) {
        fprintf(stderr, "Error: timeout must be non-negative.\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 's':
      *seed = atoi(optarg);
      break;
    case 'v':
      *view_path = optarg;
      break;
    case 'p':
      optind--;
      while (optind < argc && *num_players < MAX_PLAYERS && argv[optind][0] != '-') {
        player_paths[(*num_players)++] = argv[optind++];
      }
      break;
    default:
      fprintf(stderr,
              "Usage: %s [-w width] [-h height] [-d delay] [-t timeout] [-s seed] [-v "
              "view_path] [-p player_paths...]\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }
}

void print_parameters(int width, int height, int delay, int timeout, unsigned int seed, char *view_path, char *player_paths[], int num_players) {
  printf("Parameters:\n");
  printf("Width: %d\n", width);
  printf("Height: %d\n", height);
  printf("Delay: %d ms\n", delay);
  printf("Timeout: %d s\n", timeout);
  printf("Seed: %u\n", seed);
  printf("View path: %s\n", view_path ? view_path : "None");
  printf("Player paths:\n");
  for (int i = 0; i < num_players; i++) {
    printf("  %s\n", player_paths[i]);
  }
}