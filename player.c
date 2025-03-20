#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
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
  sem_t sem_view_ready;
  sem_t sem_master_ready;
} GameSync;

void *attach_shared_memory(const char *name, size_t size) {
  int fd = shm_open(name, O_RDWR, 0666);
  if (fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

unsigned char choose_random_move() { return rand() % 8; }

int main(int argc, char *argv[]) {
  if (argc != 3) {
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));
  GameState *state = attach_shared_memory(SHM_GAME_STATE, sizeof(GameState));
  GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync));

  while (!state->game_over) {
    sem_wait(&sync->sem_view_ready);
    // Generar un movimiento aleatorio y enviarlo al ChompChamps
    unsigned char move = choose_random_move();
    write(STDOUT_FILENO, &move,
          sizeof(move));  // El ChompChamps leerá esto por pipe

    sem_post(&sync->sem_master_ready);
    usleep(500000);  // Esperar 500ms para simular reacción del jugador
  }

  return 0;
}
