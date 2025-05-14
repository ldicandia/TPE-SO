#ifndef MASTER_CHOMP_H
#define MASTER_CHOMP_H

#include <sys/types.h>
#include <time.h>
#include "game_logic.h"
#include "shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"
#define MAX_PLAYERS 9
#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 10
#define DEFAULT_DELAY 200
#define DEFAULT_TIMEOUT 10
#define MIN_WIDTH 10
#define MIN_HEIGHT 10
#define MIN_DELAY 0
#define MIN_TIMEOUT 0
#define TIMEOUT_CHECK_INTERVAL 100000 // microseconds
#define MAX_STR_LEN 10

pid_t spawn_process(const char *path, char *width, char *height);
void check_results(int num_players, pid_t player_pids[], GameState *state,
				   pid_t view_pid);
void read_player_moves(GameState *state, GameSync *sync,
					   int player_pipes[MAX_PLAYERS][2], fd_set *readfds,
					   time_t *last_move_times, int *blocked_players,
					   int num_players, pid_t view_pid);
void initialize_shared_memory(GameState **state, GameSync **sync, int width,
							  int height, int num_players, unsigned int seed);
void cleanup_resources(int player_pipes[MAX_PLAYERS][2], int num_players,
					   GameState *state, GameSync *sync, pid_t view_pid,
					   pid_t player_pids[]);
void game_loop(GameState *state, GameSync *sync,
			   int player_pipes[MAX_PLAYERS][2], time_t *last_move_times,
			   int num_players, int timeout, int delay, pid_t view_pid);

#endif // MASTER_CHOMP_H