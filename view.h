#ifndef VIEW_H
#define VIEW_H

#include "tads/game_logic.h"
#include "tads/shmemory.h"

// Color definitions
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define GRAY "\x1b[90m"
#define ORANGE "\033[38;5;208m"
#define WHITE "\033[37m"

#define DARK_RED "\033[48;5;88m"
#define DARK_GREEN "\033[48;5;22m"
#define DARK_YELLOW "\033[48;5;94m"
#define DARK_BLUE "\033[48;5;18m"
#define DARK_MAGENTA "\033[48;5;53m"
#define DARK_CYAN "\033[48;5;30m"
#define DARK_GRAY "\033[48;5;240m"
#define DARK_ORANGE "\033[48;5;130m"
#define DARK_WHITE "\033[48;5;255m"

// Shared memory keys
#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

// Function declarations
void print_board(GameState *state);
void check_players_blocked(GameState *state);

#endif // VIEW_H