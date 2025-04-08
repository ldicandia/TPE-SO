#ifndef SHMEMORY_H
#define SHMEMORY_H

#include <semaphore.h>
#include <sys/types.h>
#include <stddef.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PLAYERS 9

typedef struct GameSync GameSync;

void *create_shared_memory(const char *name, size_t size);
void destroy_shared_memory(const char *name, void *ptr, size_t size);

void initialize_sync(GameSync *sync);
void destroy_sync(GameSync *sync);
void detach_shared_memory(void *ptr, size_t size);

void *attach_shared_memory(const char *name, size_t size, int flags, int prot);
void detach_shared_memory(void *ptr, size_t size);

void semaphore_wait(sem_t *sem);

void semaphore_post(sem_t *sem);

bool is_reader_count_zero(GameSync *sync);

void increment_reader_count(GameSync *sync);

void decrement_reader_count(GameSync *sync);

size_t get_game_sync_size(void);

void semaphore_to_view(GameSync *sync);

void semaphore_wait_mutex(GameSync *sync);

void semaphore_post_game_mutex(GameSync *sync);

void semaphore_post_state_state(GameSync *sync);

void semaphore_pre_move(GameSync *sync);

void semaphore_post_move(GameSync *sync);

void semaphore_pre_print(GameSync *sync);

void semaphore_post_print(GameSync *sync);

void semaphore_game_over(GameSync *sync);

void initialize_pipes(int player_pipes[MAX_PLAYERS][2], int num_players);

#endif // SHMEMORY_H