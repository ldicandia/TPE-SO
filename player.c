#include "tads/game_logic.h"
#include "tads/shmemory.h"
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

void *attach_shared_memory(const char *name, size_t size, int flags, int prot) {
  int fd = shm_open(name, flags, 0666); // Cambiar a O_RDONLY
  if (fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  void *ptr = mmap(NULL, size, prot, MAP_SHARED, fd, 0); // Cambiar a PROT_READ
  if (ptr == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

void detach_shared_memory(void *ptr, size_t size) {
  if (munmap(ptr, size) == -1) {
    perror("munmap");
    exit(EXIT_FAILURE);
  }
}

unsigned char choose_random_move();

int main(int argc, char *argv[]) {
  if (argc != 3) {
    exit(EXIT_FAILURE);
  }

  srand(getpid());
  GameState *state = attach_shared_memory(SHM_GAME_STATE, sizeof(GameState), O_RDONLY, PROT_READ);
  GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync), O_RDWR, PROT_READ | PROT_WRITE);

  while (!state->game_over) {
    usleep(100000);
    unsigned char move = choose_random_move();

    // if (state->players[getpid() % state->num_players].blocked) {
    //   return 0; // Salir del bucle si el jugador está bloqueado
    // }

    write(STDOUT_FILENO, &move,
          sizeof(move)); // El ChompChamps leerá esto por pipe

    sem_post(&sync->D); // Notificar que terminó su turno

    usleep(100000);
  }
  detach_shared_memory(state, sizeof(GameState));
  detach_shared_memory(sync, sizeof(GameSync));
  return 0;
}

unsigned char choose_random_move() { return rand() % 8; }