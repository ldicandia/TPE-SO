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

void initialize_sync(sem_t *sem_view_ready, sem_t *sem_master_ready,
					 sem_t *sem_state_mutex, sem_t *sem_game_mutex,
					 sem_t *sem_reader_mutex, unsigned int *reader_count) {
	sem_init(sem_view_ready, 1, 0);
	sem_init(sem_master_ready, 1, 0);
	sem_init(sem_state_mutex, 1, 1);
	sem_init(sem_game_mutex, 1, 1);
	sem_init(sem_reader_mutex, 1, 1);
	*reader_count = 0;
}

void destroy_sync(sem_t *sem_view_ready, sem_t *sem_master_ready,
				  sem_t *sem_state_mutex, sem_t *sem_game_mutex,
				  sem_t *sem_reader_mutex) {
	sem_destroy(sem_view_ready);
	sem_destroy(sem_master_ready);
	sem_destroy(sem_state_mutex);
	sem_destroy(sem_game_mutex);
	sem_destroy(sem_reader_mutex);
}

void *attach_shared_memory(const char *name, size_t size, int flags, int prot) {
	int fd = shm_open(name, flags, 0666); // Cambiar a O_RDONLY
	if (fd == -1) {
		perror("shm_open");
		exit(EXIT_FAILURE);
	}
	void *ptr =
		mmap(NULL, size, prot, MAP_SHARED, fd, 0); // Cambiar a PROT_READ
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
