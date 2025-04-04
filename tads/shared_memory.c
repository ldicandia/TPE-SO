#include "shared_memory.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

void *attach_shared_memory(const char *name, size_t size, int flags, int prot) {
  int fd = shm_open(name, flags, 0666);
  if (fd == -1) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  void *ptr = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
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

void destroy_shared_memory(const char *name) {
  if (shm_unlink(name) == -1) {
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
}