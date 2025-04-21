// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "../headers/game_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define READ_END 0
#define WRITE_END 1

struct Player {
	char name[16];
	unsigned int score;
	unsigned int invalid_moves;
	unsigned int valid_moves;
	unsigned short x, y;
	pid_t pid;
	bool blocked;
};

struct GameState {
	unsigned short width;
	unsigned short height;
	unsigned int num_players;
	Player players[MAX_PLAYERS];
	bool game_over;
	int board[];
};

void initialize_board(GameState *state, unsigned int seed) {
	srand(seed);
	for (int i = 0; i < state->width * state->height; i++) {
		state->board[i] = rand() % 9 + 1; // Random rewards 1-9
	}
}

void place_players(GameState *state) {
	for (int i = 0; i < state->num_players; i++) {
		state->players[i].x =
			(state->width / (state->num_players + 1)) * (i + 1);
		state->players[i].y				= state->height / 2;
		state->players[i].score			= 0;
		state->players[i].invalid_moves = 0;
		state->players[i].valid_moves	= 0;
		state->players[i].blocked		= false;
		snprintf(state->players[i].name, sizeof(state->players[i].name),
				 "Player%d", i + 1);
	}
}

bool is_valid_move(GameState *state, int player_idx, unsigned char move) {
	const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
	const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
	int dir			= move;
	int new_x		= state->players[player_idx].x + dx[dir];
	int new_y		= state->players[player_idx].y + dy[dir];
	return (new_x >= 0 && new_x < state->width && new_y >= 0 &&
			new_y < state->height &&
			state->board[new_y * state->width + new_x] > 0);
}

bool has_valid_moves(GameState *state, int player_idx) {
	for (int move = 0; move < 8; move++) {
		if (is_valid_move(state, player_idx, move)) {
			return true;
		}
	}
	return false;
}

void process_move(GameState *state, int player_idx, unsigned char move) {
	if (!is_valid_move(state, player_idx, move)) {
		state->players[player_idx].invalid_moves++;
	}
	else {
		const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
		const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
		int new_x		= state->players[player_idx].x + dx[move];
		int new_y		= state->players[player_idx].y + dy[move];
		state->players[player_idx].score +=
			state->board[new_y * state->width + new_x];
		state->players[player_idx].x = new_x;
		state->players[player_idx].y = new_y;
		state->players[player_idx].valid_moves++;
		state->board[new_y * state->width + new_x] =
			0 - player_idx; // Cell consumed
	}
}

size_t get_game_state_size() {
	return sizeof(GameState);
}

void set_width(GameState *state, int width) {
	state->width = width;
}

void set_height(GameState *state, int height) {
	state->height = height;
}

void set_num_players(GameState *state, int num_players) {
	state->num_players = num_players;
}

void set_game_over(GameState *state, bool game_over) {
	state->game_over = game_over;
}

void set_player_pid(GameState *state, int player_idx, pid_t pid) {
	state->players[player_idx].pid = pid;
}

bool is_game_over(const GameState *state) {
	return state->game_over;
}

bool is_player_blocked(GameState *state, int player_idx) {
	return state->players[player_idx].blocked;
}

void set_player_blocked(GameState *state, int player_idx, bool blocked) {
	state->players[player_idx].blocked = blocked;
}

int get_player_score(GameState *state, int player_idx) {
	return state->players[player_idx].score;
}

int get_num_players(const GameState *state) {
	return state->num_players;
}

char *get_player_name(GameState *state, int player_idx) {
	return state->players[player_idx].name;
}

int get_player_x(GameState *state, int player_idx) {
	return state->players[player_idx].x;
}

int get_player_y(GameState *state, int player_idx) {
	return state->players[player_idx].y;
}

int get_width(const GameState *state) {
	return state->width;
}

int get_height(const GameState *state) {
	return state->height;
}

int get_state_value(GameState *state, int x, int y) {
	return state->board[y * state->width + x];
}

int get_player_pid(GameState *state, int player_idx) {
	return state->players[player_idx].pid;
}

void check_player_timeouts(GameState *state, const time_t *last_move_times,
						   int timeout, int *blocked_players, int num_players) {
	time_t current_time = time(NULL);
	for (int i = 0; i < num_players; i++) {
		if (!is_player_blocked(state, i)) {
			if (!has_valid_moves(state, i)) {
				set_player_blocked(state, i, true);
				(*blocked_players)++;
			}
			else if (current_time - last_move_times[i] >= timeout) {
				set_player_blocked(state, i, true);
				(*blocked_players)++;
			}
		}
	}
}

void spawn_players(GameState *state, int player_pipes[MAX_PLAYERS][2],
				   char *player_paths[], int num_players, char *width_str,
				   char *height_str, pid_t player_pids[MAX_PLAYERS]) {
	for (int i = 0; i < num_players; i++) {
		player_pids[i] = fork();
		if (player_pids[i] == 0) {
			dup2(player_pipes[i][1], STDOUT_FILENO);
			close(player_pipes[i][0]);
			close(player_pipes[i][1]);

			for (int j = 0; j < num_players; j++) {
				if (j != i) {
					close(player_pipes[j][READ_END]);
					close(player_pipes[j][WRITE_END]);
				}
			}

			execl(player_paths[i], player_paths[i], width_str, height_str,
				  NULL);
			perror("execl");
			exit(EXIT_FAILURE);
		}
		set_player_pid(state, i, player_pids[i]);
	}
}
