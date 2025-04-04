#include "game_sync.h"
#include <stdlib.h>

GameSync *create_game_sync() {
  GameSync *sync = malloc(sizeof(GameSync));
  if (!sync) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  sem_init(&sync->A, 1, 0);
  sem_init(&sync->B, 1, 0);
  sem_init(&sync->C, 1, 1);
  sem_init(&sync->D, 1, 1);
  sem_init(&sync->E, 1, 1);
  sync->F = 0;
  return sync;
}

void destroy_game_sync(GameSync *sync) {
  sem_destroy(&sync->A);
  sem_destroy(&sync->B);
  sem_destroy(&sync->C);
  sem_destroy(&sync->D);
  sem_destroy(&sync->E);
  free(sync);
}