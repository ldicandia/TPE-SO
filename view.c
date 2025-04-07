#include <ncurses.h>
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

#include <locale.h>
#include <wchar.h>

#include "tads/game_logic.h"
#include "tads/shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

// Function prototypes
void display_game(GameState *state);
void cleanup_ncurses();

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, ""); // Enable UTF-8 support

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    size_t game_state_size = sizeof(GameState) + width * height * sizeof(int);

    GameState *state = attach_shared_memory(SHM_GAME_STATE, game_state_size, O_RDONLY, PROT_READ);
    GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync), O_RDWR, PROT_READ | PROT_WRITE);

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0); // Hide the cursor
    start_color(); // Enable colors

    // Initialize color pairs for players and board
    for (int i = 0; i < 8; i++) {
        init_pair(i + 1, i + 1, COLOR_BLACK); // Player colors
        init_pair(i + 9, i + 1, COLOR_BLACK); // Darker colors for blocked players
    }
    init_pair(17, COLOR_WHITE, COLOR_BLACK); // White for empty cells

    while (true) {
        if (state->game_over) {
            sem_post(&sync->sem_master_ready); // Release the master if waiting
            break;
        }
        sem_wait(&sync->sem_view_ready);
        display_game(state);
        sem_post(&sync->sem_master_ready);
    }

    cleanup_ncurses();
    return 0;
}

void display_game(GameState *state) {
    clear(); // Clear the screen

    // Header
    mvprintw(0, 0, "=== 🟢 ChompChamps — Game Status ===");

    // Player information
    mvprintw(2, 0, "👥 Players Info:");
    for (int i = 0; i < state->num_players; i++) {
        const char *status = state->players[i].blocked ? "🚫 Blocked" : "✅ Active";
        attron(COLOR_PAIR(i + 1)); // Use the player's color
        mvprintw(3 + i, 0, "[%s] %s - Pos: (%d,%d), Score: %d", state->players[i].name, status,
                 state->players[i].x, state->players[i].y, state->players[i].score);
        attroff(COLOR_PAIR(i + 1));
    }

    // Board header
    int board_start_y = 5 + state->num_players;
    int board_start_x = 3;
    mvprintw(board_start_y - 2, board_start_x, "🧩 Board (%dx%d):", state->width, state->height);

    // Column headers
    mvprintw(board_start_y - 1, board_start_x + 3, " ");
    for (int x = 0; x < state->width; x++) {
        mvprintw(board_start_y - 1, board_start_x + 3 + x * 3, "%2d", x);
    }

    // Top border
    mvprintw(board_start_y, board_start_x, "   ");
    for (int x = 0; x < state->width; x++) {
        mvprintw(board_start_y, board_start_x + 3 + x * 3, "───");
    }

    // Board content
    for (int y = 0; y < state->height; y++) {
        mvprintw(board_start_y + 1 + y, board_start_x, "%2d│", y); // Row header
        for (int x = 0; x < state->width; x++) {
            bool printed = false;

            // Check if a player is on this cell
            for (int i = 0; i < state->num_players; i++) {
                if (state->players[i].x == x && state->players[i].y == y) {
                    // Snake head
                    attron(COLOR_PAIR(i + 1)); // Use the player's color
                    mvaddstr(board_start_y + 1 + y, board_start_x + 3 + x * 3, " █ "); // Larger square for the head
                    attroff(COLOR_PAIR(i + 1));
                    printed = true;
                    break;
                }
            }

            if (!printed) {
                int value = state->board[y * state->width + x];
                if (value < 0 && -value <= state->num_players) {
                    // Snake body
                    attron(COLOR_PAIR(-value)); // Use the color of the snake that consumed the square
                    mvaddstr(board_start_y + 1 + y, board_start_x + 3 + x * 3, " ■ ");
                    attroff(COLOR_PAIR(-value));
                } else if (value > 0) {
                    // Display the number in white for unconsumed cells
                    attron(COLOR_PAIR(17)); // White for numbers
                    mvprintw(board_start_y + 1 + y, board_start_x + 3 + x * 3, " %d ", value);
                    attroff(COLOR_PAIR(17));
                } else {
                    // Empty cell
                    attron(COLOR_PAIR(17)); // White for empty cells
                    mvaddstr(board_start_y + 1 + y, board_start_x + 3 + x * 3, " . ");
                    attroff(COLOR_PAIR(17));
                }
            }
        }
        mvprintw(board_start_y + 1 + y, board_start_x + 3 + state->width * 3, "│");
    }

    // Bottom border
    mvprintw(board_start_y + 1 + state->height, board_start_x, "   ");
    for (int x = 0; x < state->width; x++) {
        mvprintw(board_start_y + 1 + state->height, board_start_x + 3 + x * 3, "───");
    }

    // Scores
    int scoreboard_start_y = board_start_y + 3 + state->height;
    mvprintw(scoreboard_start_y, board_start_x, "🏆 Scores:");
    for (int i = 0; i < state->num_players; i++) {
        attron(COLOR_PAIR(i + 1)); // Use the player's color
        mvprintw(scoreboard_start_y + i + 1, board_start_x, "%s: %d", state->players[i].name, state->players[i].score);
        attroff(COLOR_PAIR(i + 1));
    }

    refresh(); // Refresh the screen to show changes
}

void cleanup_ncurses() {
    endwin(); // Restore terminal to normal mode
}