#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define MAX_PLAYERS 9

typedef struct {
	char name[16];
	unsigned int score;
	unsigned int invalid_moves;
	unsigned int valid_moves;
	unsigned short x, y;
	pid_t pid;
	bool blocked;
} Player;

typedef struct {
	unsigned short width;
	unsigned short height;
	unsigned int num_players;
	Player players[MAX_PLAYERS];
	bool game_over;
	int board[];
} GameState;

void initialize_board(GameState *state, unsigned int seed);

void place_players(GameState *state);

bool is_valid_move(GameState *state, int player_idx, unsigned char move);

bool has_valid_moves(GameState *state, int player_idx);

void process_move(GameState *state, int player_idx, unsigned char move);

#endif // GAME_LOGIC_H