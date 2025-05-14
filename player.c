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

#include "headers/game_logic.h"
#include "headers/shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

unsigned char choose_random_move();

int main(int argc, char *argv[]) {
  if (argc != 3)
    exit(EXIT_FAILURE);
  srand(getpid());

  GameState *state = attach_shared_memory(SHM_GAME_STATE, get_game_state_size(),
                                          O_RDONLY, PROT_READ);
  GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, get_game_sync_size(),
                                        O_RDWR, PROT_READ | PROT_WRITE);

  pid_t my_pid = getpid();
  int player_idx = -1;
  int num_players = get_num_players(state);
  while (player_idx < 0) {
    semaphore_wait_and_post_mutex(sync);
    semaphore_pre_move(sync);
    for (int i = 0; i < num_players; i++) {
      if (get_player_pid(state, i) == my_pid) {
        player_idx = i;
        break;
      }
    }
    semaphore_post_move(sync);
  }

  int last_sum = -1;
  int current_sum;
  bool over = false;

  while (!is_game_over(state)) {
    semaphore_wait_and_post_mutex(sync);
    do {
      semaphore_pre_move(sync);
      current_sum = get_players_valid_moves(state, player_idx) +
                    get_players_invalid_moves(state, player_idx);
      over = is_game_over(state);

      semaphore_post_move(sync);
    } while (!over && current_sum == last_sum);

    if (over)
      break;
    last_sum = current_sum;
    unsigned char move = choose_random_move();
    write(STDOUT_FILENO, &move, sizeof(move));
  }

  detach_shared_memory(state, get_game_state_size());
  detach_shared_memory(sync, get_game_sync_size());
  return 0;
}

unsigned char choose_random_move() { return rand() % 8; }
