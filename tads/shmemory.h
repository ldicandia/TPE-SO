#ifndef SHMEMORY_H
#define SHMEMORY_H

#include <semaphore.h>
#include <stddef.h>
// Definición de la estructura GameSync
typedef struct {
  sem_t sem_view_ready;        // Se usa para indicarle a la vista que hay cambios por imprimir
  sem_t sem_master_ready;        // Se usa para indicarle al master que la vista terminó de imprimir
  sem_t sem_state_mutex;        // Mutex para evitar inanición del master al acceder al estado
  sem_t sem_game_mutex;        // Mutex para el estado del juego
  sem_t sem_reader_mutex;        // Mutex para la siguiente variable
  unsigned int reader_count; // Cantidad de jugadores leyendo el estado
} GameSync;

void *create_shared_memory(const char *name, size_t size);
void destroy_shared_memory(const char *name, void *ptr, size_t size);

void initialize_sync(sem_t *sem_view_ready, sem_t *sem_master_ready, sem_t *sem_state_mutex, sem_t *sem_game_mutex, sem_t *sem_reader_mutex, unsigned int *reader_count);
void destroy_sync(sem_t *sem_view_ready, sem_t *sem_master_ready, sem_t *sem_state_mutex, sem_t *sem_game_mutex, sem_t *sem_reader_mutex);
void detach_shared_memory(void *ptr, size_t size);

void *attach_shared_memory(const char *name, size_t size, int flags, int prot);
void detach_shared_memory(void *ptr, size_t size);

#endif // SHMEMORY_H