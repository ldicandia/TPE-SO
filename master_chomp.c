// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "tads/game_logic.h"
#include "tads/shmemory.h"

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
void parse_arguments(int argc, char *argv[], int *width, int *height,
					 int *delay, int *timeout, unsigned int *seed,
					 char **view_path, char *player_paths[], int *num_players);
void print_parameters(int width, int height, int delay, int timeout,
					  unsigned int seed, char *view_path, char *player_paths[],
					  int num_players);
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

int main(int argc, char *argv[]) {
	int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT, delay = DEFAULT_DELAY,
		timeout		  = DEFAULT_TIMEOUT;
	unsigned int seed = time(NULL);
	char *view_path	  = NULL;
	char *player_paths[MAX_PLAYERS];
	int num_players = 0;

	parse_arguments(argc, argv, &width, &height, &delay, &timeout, &seed,
					&view_path, player_paths, &num_players);
	print_parameters(width, height, delay, timeout, seed, view_path,
					 player_paths, num_players);

	if (num_players < 1 || num_players > MAX_PLAYERS) {
		fprintf(stderr, "Error: Number of players must be between 1 and %d\n",
				MAX_PLAYERS);
		exit(EXIT_FAILURE);
	}

	GameState *state;
	GameSync *sync;
	initialize_shared_memory(&state, &sync, width, height, num_players, seed);

	int player_pipes[MAX_PLAYERS][2];
	initialize_pipes(player_pipes, num_players);

	char width_str[MAX_STR_LEN], height_str[MAX_STR_LEN];
	sprintf(width_str, "%d", width);
	sprintf(height_str, "%d", height);

	pid_t view_pid =
		view_path ? spawn_process(view_path, width_str, height_str) : 0;

	pid_t player_pids[MAX_PLAYERS];
	spawn_players(state, player_pipes, player_paths, num_players, width_str,
				  height_str, player_pids);

	time_t *last_move_times = malloc(num_players * sizeof(time_t));
	if (last_move_times == NULL) {
		fprintf(stderr,
				"Error: Memory allocation failed for last_move_times.\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < num_players; i++) {
		last_move_times[i] = time(NULL);
	}

	game_loop(state, sync, player_pipes, last_move_times, num_players, timeout,
			  delay, view_pid);

	free(last_move_times);
	cleanup_resources(player_pipes, num_players, state, sync, view_pid,
					  player_pids);

	return 0;
}

pid_t spawn_process(const char *path, char *width, char *height) {
	pid_t pid = fork();
	if (pid == 0) {
		execl(path, path, width, height, NULL);
		perror("execl");
		exit(EXIT_FAILURE);
	}
	return pid;
}

void check_results(int num_players, pid_t player_pids[], GameState *state,
				   pid_t view_pid) {
	for (int i = 0; i < num_players; i++) {
		int status;
		waitpid(player_pids[i], &status, 0);
		if (WIFEXITED(status)) {
			printf("Player %d (PID: %d)|| Exit status %d || Score = %u\n",
				   i + 1, player_pids[i], WEXITSTATUS(status),
				   get_player_score(state, i));
		}
		else if (WIFSIGNALED(status)) {
			printf("Player %d (PID: %d)|| Killed by signal %d\n", i + 1,
				   player_pids[i], WTERMSIG(status));
		}
		else if (WIFSTOPPED(status)) {
			printf("Player %d (PID: %d)|| Stopped by signal %d\n", i + 1,
				   player_pids[i], WSTOPSIG(status));
		}
		else {
			printf("Player %d (PID: %d)|| Unknown status\n", i, player_pids[i]);
		}
	}

	if (view_pid) {
		int status;
		waitpid(view_pid, &status, 0);
		printf("View (PID %d) exited with status %d\n", view_pid, status);
	}

	printf("Master Chomp PID: %d\n", getpid());
}

void parse_arguments(int argc, char *argv[], int *width, int *height,
					 int *delay, int *timeout, unsigned int *seed,
					 char **view_path, char *player_paths[], int *num_players) {
	int opt;
	while ((opt = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
		switch (opt) {
			case 'w':
				*width = atoi(optarg);
				if (*width < MIN_WIDTH) {
					fprintf(stderr, "Error: minimum width must be %d.\n",
							MIN_WIDTH);
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				*height = atoi(optarg);
				if (*height < MIN_HEIGHT) {
					fprintf(stderr, "Error: minimum height must be %d.\n",
							MIN_HEIGHT);
					exit(EXIT_FAILURE);
				}
				break;
			case 'd':
				*delay = atoi(optarg);
				if (*delay < MIN_DELAY) {
					fprintf(stderr, "Error: delay must be non-negative.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 't':
				*timeout = atoi(optarg);
				if (*timeout < MIN_TIMEOUT) {
					fprintf(stderr, "Error: timeout must be non-negative.\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 's':
				*seed = atoi(optarg);
				break;
			case 'v':
				*view_path = optarg;
				break;
			case 'p':
				optind--;
				while (optind < argc && *num_players < MAX_PLAYERS &&
					   argv[optind][0] != '-') {
					player_paths[(*num_players)++] = argv[optind++];
				}
				break;
			default:
				fprintf(stderr,
						"Usage: %s [-w width] [-h height] [-d delay] [-t "
						"timeout] [-s seed] [-v "
						"view_path] [-p player_paths...]\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}
}

void print_parameters(int width, int height, int delay, int timeout,
					  unsigned int seed, char *view_path, char *player_paths[],
					  int num_players) {
	printf("Parameters:\n");
	printf("Width: %d\n", width);
	printf("Height: %d\n", height);
	printf("Delay: %d ms\n", delay);
	printf("Timeout: %d s\n", timeout);
	printf("Seed: %u\n", seed);
	printf("View path: %s\n", view_path ? view_path : "None");
	printf("Player paths:\n");
	for (int i = 0; i < num_players; i++) {
		printf("  %s\n", player_paths[i]);
	}
}

void read_player_moves(GameState *state, GameSync *sync,
					   int player_pipes[MAX_PLAYERS][2], fd_set *readfds,
					   time_t *last_move_times, int *blocked_players,
					   int num_players, pid_t view_pid) {
	for (int i = 0; i < num_players; i++) {
		if (!is_player_blocked(state, i) &&
			FD_ISSET(player_pipes[i][0], readfds)) {
			unsigned char move;
			ssize_t bytes_read = read(player_pipes[i][0], &move, sizeof(move));
			if (bytes_read <= 0) {
				set_player_blocked(state, i, true);
				(*blocked_players)++;
			}
			else {
				semaphore_wait_mutex(sync);
				process_move(state, i, move);

				semaphore_post_game_mutex(sync);
				if (view_pid) {
					semaphore_to_view(sync);
				}
				semaphore_post_state_state(sync);

				last_move_times[i] = time(NULL);
			}
		}
	}
}

void initialize_shared_memory(GameState **state, GameSync **sync, int width,
							  int height, int num_players, unsigned int seed) {
	*state = create_shared_memory(
		SHM_GAME_STATE, get_game_state_size() + (width * height * sizeof(int)));
	*sync = create_shared_memory(SHM_GAME_SYNC, get_game_sync_size());

	initialize_sync(*sync);

	set_width(*state, width);
	set_height(*state, height);
	set_num_players(*state, num_players);
	set_game_over(*state, false);
	set_player_pid(*state, 0, getpid());
	set_player_blocked(*state, 0, false);

	initialize_board(*state, seed);
	place_players(*state);
}

void cleanup_resources(int player_pipes[MAX_PLAYERS][2], int num_players,
					   GameState *state, GameSync *sync, pid_t view_pid,
					   pid_t player_pids[]) {
	for (int i = 0; i < num_players; i++) {
		close(player_pipes[i][0]);
	}

	check_results(num_players, player_pids, state, view_pid);

	destroy_sync(sync);
	destroy_shared_memory(SHM_GAME_STATE, state, get_game_state_size());
	destroy_shared_memory(SHM_GAME_SYNC, sync, get_game_sync_size());
}

void game_loop(GameState *state, GameSync *sync,
			   int player_pipes[MAX_PLAYERS][2], time_t *last_move_times,
			   int num_players, int timeout, int delay, pid_t view_pid) {
	fd_set readfds;
	struct timeval tv;
	int max_fd = -1;
	for (int i = 0; i < num_players; i++) {
		if (player_pipes[i][0] > max_fd)
			max_fd = player_pipes[i][0];
	}

	int blocked_players = 0;
	while (!is_game_over(state)) {
		check_player_timeouts(state, last_move_times, timeout, &blocked_players,
							  num_players);

		if (blocked_players == num_players) {
			set_game_over(state, true);
			if (view_pid) {
				semaphore_to_view(sync);
			}
		}

		FD_ZERO(&readfds);
		for (int i = 0; i < num_players; i++) {
			if (!is_player_blocked(state, i)) {
				FD_SET(player_pipes[i][0], &readfds);
			}
		}

		tv.tv_sec  = 0;
		tv.tv_usec = TIMEOUT_CHECK_INTERVAL;

		int ready = select(max_fd + 1, &readfds, NULL, NULL, &tv);
		if (ready == -1) {
			perror("select");
			break;
		}

		read_player_moves(state, sync, player_pipes, &readfds, last_move_times,
						  &blocked_players, num_players, view_pid);

		if (view_pid) {
			usleep(delay * 1000);
		}
	}
}
