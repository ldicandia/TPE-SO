#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define MAX_PLAYERS 9

typedef struct GameState GameState;
typedef struct Player Player;

void initialize_board(GameState *state, unsigned int seed);

void place_players(GameState *state);

bool is_valid_move(GameState *state, int player_idx, unsigned char move);

bool has_valid_moves(GameState *state, int player_idx);

void process_move(GameState *state, int player_idx, unsigned char move);

void set_width(GameState *state, int width);

void set_height(GameState *state, int height);

void set_num_players(GameState *state, int num_players);

void set_game_over(GameState *state, bool game_over);

void set_player_pid(GameState *state, int player_idx, pid_t pid);

bool is_game_over(const GameState *state);

void set_game_over(GameState *state, bool game_over);

bool is_player_blocked(GameState *state, int player_idx);

void set_player_blocked(GameState *state, int player_idx, bool blocked);

int get_player_score(GameState *state, int player_idx);

size_t get_game_state_size();

int get_num_players(const GameState *state);

char *get_player_name(GameState *state, int player_idx);

int get_player_x(GameState *state, int player_idx);

int get_player_y(GameState *state, int player_idx);

int get_width(const GameState *state);

int get_height(const GameState *state);

int get_state_value(GameState *state, int x, int y);

int get_player_pid(GameState *state, int player_idx);

int get_players_valid_moves(GameState *state, int player_idx);

int get_players_invalid_moves(GameState *state, int player_idx);

void check_player_timeouts(GameState *state, const time_t *last_move_times,
                           int timeout, int *blocked_players, int num_players);

void spawn_players(GameState *state, int player_pipes[MAX_PLAYERS][2],
                   char *player_paths[], int num_players, char *width_str,
                   char *height_str, pid_t player_pids[MAX_PLAYERS]);

#endif // GAME_LOGIC_H