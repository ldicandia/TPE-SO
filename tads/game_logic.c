#include "game_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialize_board(GameState *state, unsigned int seed) {
  srand(seed);
  for (int i = 0; i < state->width * state->height; i++) {
    state->board[i] = rand() % 9 + 1; // Random rewards 1-9
  }
}

void place_players(GameState *state) {
  for (int i = 0; i < state->num_players; i++) {
    state->players[i].x = (state->width / (state->num_players + 1)) * (i + 1);
    state->players[i].y = state->height / 2;
    state->players[i].score = 0;
    state->players[i].invalid_moves = 0;
    state->players[i].valid_moves = 0;
    state->players[i].blocked = false;
    snprintf(state->players[i].name, 16, "Player%d", i + 1);
  }
}

bool is_valid_move(GameState *state, int player_idx, unsigned char move) {
  int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
  int dir = move;
  int new_x = state->players[player_idx].x + dx[dir];
  int new_y = state->players[player_idx].y + dy[dir];
  return (new_x >= 0 && new_x < state->width && new_y >= 0 && new_y < state->height && state->board[new_y * state->width + new_x] > 0);
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
  } else {
    int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int new_x = state->players[player_idx].x + dx[move];
    int new_y = state->players[player_idx].y + dy[move];
    state->players[player_idx].score += state->board[new_y * state->width + new_x];
    state->players[player_idx].x = new_x;
    state->players[player_idx].y = new_y;
    state->players[player_idx].valid_moves++;
    state->board[new_y * state->width + new_x] = 0 - player_idx; // Cell consumed
  }
}