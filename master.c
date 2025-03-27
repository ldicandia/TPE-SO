#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

typedef struct {
  char name[16];
  unsigned int score;
  unsigned int invalid_moves;
  unsigned int valid_moves;
  unsigned short x, y;
  pid_t pid;
  bool active;
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
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  unsigned short width = atoi(argv[1]);
  unsigned short height = atoi(argv[2]);

  // Crear memoria compartida para el estado del juego
  size_t game_state_size = sizeof(GameState) + width * height * sizeof(int);
  GameState *state = create_shared_memory(SHM_GAME_STATE, game_state_size);
  state->width = width;
  state->height = height;
  state->num_players = 2;  // Por ejemplo, 2 jugadores
  state->game_over = false;

  return 0;
}