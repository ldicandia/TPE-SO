#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>

typedef struct {
  sem_t A;
  sem_t B;
  sem_t C;
  sem_t D;
  sem_t E;
  unsigned int F;
} GameSync;

// Functions
GameSync *create_game_sync();
void destroy_game_sync(GameSync *sync);

#endif