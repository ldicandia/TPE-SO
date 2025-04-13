// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

	GameState *state = attach_shared_memory(
		SHM_GAME_STATE, get_game_state_size(), O_RDONLY, PROT_READ);
	GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, get_game_sync_size(),
										  O_RDWR, PROT_READ | PROT_WRITE);

	while (!is_game_over(state)) {
		usleep(200000);

		semaphore_pre_move(sync);
		unsigned char move = choose_random_move();
		write(STDOUT_FILENO, &move, sizeof(move));
		semaphore_post_move(sync);
	}

	detach_shared_memory(state, get_game_state_size());
	detach_shared_memory(sync, get_game_sync_size());
	return 0;
}

unsigned char choose_random_move() {
	return rand() % 8;
}
