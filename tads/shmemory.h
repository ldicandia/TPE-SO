#ifndef SHMEMORY_H
#define SHMEMORY_H

#include <semaphore.h>
#include <stddef.h>
// Definición de la estructura GameSync
typedef struct {
  sem_t A;        // Se usa para indicarle a la vista que hay cambios por imprimir
  sem_t B;        // Se usa para indicarle al master que la vista terminó de imprimir
  sem_t C;        // Mutex para evitar inanición del master al acceder al estado
  sem_t D;        // Mutex para el estado del juego
  sem_t E;        // Mutex para la siguiente variable
  unsigned int F; // Cantidad de jugadores leyendo el estado
} GameSync;

void *create_shared_memory(const char *name, size_t size);
void destroy_shared_memory(const char *name, void *ptr, size_t size);

void initialize_sync(sem_t *A, sem_t *B, sem_t *C, sem_t *D, sem_t *E, unsigned int *F);
void destroy_sync(sem_t *A, sem_t *B, sem_t *C, sem_t *D, sem_t *E);
void detach_shared_memory(void *ptr, size_t size);

void *attach_shared_memory(const char *name, size_t size, int flags, int prot);
void detach_shared_memory(void *ptr, size_t size);

#endif // SHMEMORY_H