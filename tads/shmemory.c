#include "shmemory.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

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

void destroy_shared_memory(const char *name, void *ptr, size_t size) {
  munmap(ptr, size);
  shm_unlink(name);
}

void initialize_sync(sem_t *A, sem_t *B, sem_t *C, sem_t *D, sem_t *E, unsigned int *F) {
  sem_init(A, 1, 0);
  sem_init(B, 1, 0);
  sem_init(C, 1, 1);
  sem_init(D, 1, 1);
  sem_init(E, 1, 1);
  *F = 0;
}

void destroy_sync(sem_t *A, sem_t *B, sem_t *C, sem_t *D, sem_t *E) {
  sem_destroy(A);
  sem_destroy(B);
  sem_destroy(C);
  sem_destroy(D);
  sem_destroy(E);
}