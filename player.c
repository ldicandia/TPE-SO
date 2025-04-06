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

#include "tads/game_logic.h"
#include "tads/shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

unsigned char choose_random_move();

int main(int argc, char *argv[]) {
  if (argc != 3) {
    exit(EXIT_FAILURE);
  }

  srand(getpid());

  GameState *state = attach_shared_memory(SHM_GAME_STATE, sizeof(GameState), O_RDONLY, PROT_READ);
  GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync), O_RDWR, PROT_READ | PROT_WRITE);

  while (!state->game_over) {
    usleep(200000);

    sem_wait(&sync->sem_game_mutex);
    sem_post(&sync->sem_game_mutex);

    sem_wait(&sync->sem_state_mutex);
    if (sync->reader_count++ == 0) {
      sem_wait(&sync->sem_game_mutex);
    }
    sem_post(&sync->sem_state_mutex);

    unsigned char move = choose_random_move();
    write(STDOUT_FILENO, &move, sizeof(move));

    sem_wait(&sync->sem_state_mutex);
    if (--sync->reader_count == 0) {
      sem_post(&sync->sem_game_mutex);
    }
    sem_post(&sync->sem_state_mutex);

    sem_post(&sync->sem_game_mutex);
  }
  detach_shared_memory(state, sizeof(GameState));
  detach_shared_memory(sync, sizeof(GameSync));
  return 0;
}

unsigned char choose_random_move() { return rand() % 8; }
