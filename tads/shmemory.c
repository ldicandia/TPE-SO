// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "shmemory.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

struct GameSync {
	sem_t sem_view_ready;	// Se usa para indicarle a la vista que hay cambios
							// por imprimir
	sem_t sem_master_ready; // Se usa para indicarle al master que la vista
							// terminó de imprimir
	sem_t sem_state_mutex;	// Mutex para evitar inanición del master al acceder
							// al estado
	sem_t sem_game_mutex;	// Mutex para el estado del juego
	sem_t sem_reader_mutex; // Mutex para la siguiente variable
	unsigned int reader_count; // Cantidad de jugadores leyendo el estado
};

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

void initialize_sync(GameSync *sync) {
	sync->reader_count = 0;
	sem_init(&sync->sem_view_ready, 1, 0);
	sem_init(&sync->sem_master_ready, 1, 0);
	sem_init(&sync->sem_state_mutex, 1, 1);
	sem_init(&sync->sem_game_mutex, 1, 1);
	sem_init(&sync->sem_reader_mutex, 1, 1);
}

void destroy_sync(GameSync *sync) {
	sem_destroy(&sync->sem_view_ready);
	sem_destroy(&sync->sem_master_ready);
	sem_destroy(&sync->sem_state_mutex);
	sem_destroy(&sync->sem_game_mutex);
	sem_destroy(&sync->sem_reader_mutex);
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

void semaphore_wait(sem_t *sem) {
	if (sem_wait(sem) == -1) {
		perror("sem_wait");
		exit(EXIT_FAILURE);
	}
}

void semaphore_post(sem_t *sem) {
	if (sem_post(sem) == -1) {
		perror("sem_post");
		exit(EXIT_FAILURE);
	}
}

bool is_reader_count_zero(GameSync *sync) {
	return sync->reader_count == 0;
}
void increment_reader_count(GameSync *sync) {
	sync->reader_count++;
}
void decrement_reader_count(GameSync *sync) {
	sync->reader_count--;
}

size_t get_game_sync_size(void) {
	return sizeof(GameSync);
}

void semaphore_to_view(GameSync *sync) {
	semaphore_post(&sync->sem_view_ready);
	semaphore_wait(&sync->sem_master_ready);
}

void semaphore_wait_mutex(GameSync *sync) {
	semaphore_wait(&sync->sem_state_mutex);
	semaphore_wait(&sync->sem_game_mutex);
}

void semaphore_post_game_mutex(GameSync *sync) {
	semaphore_post(&sync->sem_game_mutex);
}

void semaphore_post_state_state(GameSync *sync) {
	semaphore_post(&sync->sem_state_mutex);
}

void semaphore_pre_move(GameSync *sync) {
	semaphore_wait(&sync->sem_game_mutex);
	semaphore_post(&sync->sem_game_mutex);

	semaphore_wait(&sync->sem_state_mutex);
	if (sync->reader_count++ == 0) {
		semaphore_wait(&sync->sem_game_mutex);
	}
	semaphore_post(&sync->sem_state_mutex);
}

void semaphore_post_move(GameSync *sync) {
	semaphore_wait(&sync->sem_state_mutex);
	if (--sync->reader_count == 0) {
		semaphore_post(&sync->sem_game_mutex);
	}
	semaphore_post(&sync->sem_state_mutex);

	semaphore_post(&sync->sem_game_mutex);
}

void semaphore_game_over(GameSync *sync) {
	sem_post(&sync->sem_master_ready);
}

void semaphore_pre_print(GameSync *sync) {
	sem_wait(&sync->sem_view_ready);
}

void semaphore_post_print(GameSync *sync) {
	sem_post(&sync->sem_master_ready);
}
