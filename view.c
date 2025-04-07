#include <fcntl.h>
#include <ncurses.h>
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
void display_welcome_message();
void display_game_over_message(GameState *state);
bool all_players_blocked(GameState *state);

void display_welcome_message() {
  clear(); // Clear the screen
  mvprintw(10, 10, "Welcome to Chomp Champs!");
  mvprintw(12, 10, "Press any key to start...");
  refresh(); // Refresh the screen to show changes
  getch();   // Wait for user input
}

void display_game_over_message(GameState *state) {
  clear(); // Clear the screen

  // Print the "Game Over" sign
  mvprintw(5, 10, " ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗ ");
  mvprintw(6, 10, "██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗");
  mvprintw(7, 10, "██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  █████╔╝");
  mvprintw(8, 10, "██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║██║   ██║██╔══╝  ██╔███╔═╝ ");
  mvprintw(9, 10, "╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝╚██████╔╝███████╗██║ ███║     ");
  mvprintw(10, 10, " ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═════╝ ╚══════╝╚═╝ ╚══╝    ");

  /*TODO -- cosas q rompi

  -El color de la cabeza es el correcto de cada serpiente, el cuerpo esta saliendo de otro color, creo q incluso hay una que sale con puntitos nada q ver
  - la cruz para las serpientes muertas
  - la pantalla de game over  */

  // Print players' results below the "Game Over" sign
  int start_y = 12; // Start printing below the "Game Over" sign
  int start_x = 15; // Indent for better alignment
  mvprintw(start_y, start_x, "Players' Results:");
  for (int i = 0; i < state->num_players; i++) {
    // mvprintw(start_y + i + 2, start_x, "Player %d || Exit status: %d || Score: %d", i + 1, state->players[i].blocked ? 1 : 0, state->players[i].score);
  }

  refresh(); // Refresh the screen to show changes
  getch();   // Wait for user input before exiting
}

bool all_players_blocked(GameState *state) {
  for (int i = 0; i < state->num_players; i++) {
    if (!state->players[i].blocked) {
      return false; // At least one player is not blocked
    }
  }
  return true; // All players are blocked
}

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
  curs_set(0);   // Hide the cursor
  start_color(); // Enable colors

  // Initialize color pairs for players and board
  for (int i = 0; i < 8; i++) {
    init_pair(i + 1, i + 1, COLOR_BLACK); // Player colors
    init_pair(i + 9, i + 1, COLOR_BLACK); // Darker colors for blocked players
  }
  init_pair(17, COLOR_WHITE, COLOR_BLACK); // White for empty cells

  // Display welcome message
  display_welcome_message();

  while (true) {
    if (state->game_over || all_players_blocked(state)) {
      break; // Exit the loop if the game is over or all players are blocked
    }
    sem_wait(&sync->sem_view_ready);
    display_game(state);
    sem_post(&sync->sem_master_ready);
  }

  // Display game over message
  display_game_over_message(state);

  sleep(3);

  cleanup_ncurses();
  return 0;
}
void display_game(GameState *state) {
  clear(); // Clear the screen

  int rows, cols;
  getmaxyx(stdscr, rows, cols); // Get terminal dimensions

  // Center the board and other elements
  int board_start_y = (rows - (state->height + 3)) / 2;    // Center vertically
  int board_start_x = (cols - (state->width * 4 + 2)) / 2; // Center horizontally

  // Cool Header for "Chomp Champs"
  mvprintw(board_start_y - 6, (cols - 70) / 2, " ██████╗██╗  ██╗ ██████╗ ███╗   ███╗██████╗      ██████╗██╗  ██╗ █████╗ ███╗   ███╗██████╗");
  mvprintw(board_start_y - 5, (cols - 70) / 2, "██╔════╝██║  ██║██╔═══██╗████╗ ████║██╔══██╗    ██╔════╝██║  ██║██╔══██╗████╗ ████║██╔══██╗");
  mvprintw(board_start_y - 4, (cols - 70) / 2, "██║     ███████║██║   ██║██╔████╔██║██████╔╝    ██║     ███████║███████║██╔████╔██║██████╔╝");
  mvprintw(board_start_y - 3, (cols - 70) / 2, "██║     ██╔══██║██║   ██║██╔╝██╔╝██║██╔═══╝     ██║     ██╔══██║██╔══██║██╔╝██╔╝██║██╔═══╝");
  mvprintw(board_start_y - 2, (cols - 70) / 2, "╚██████╗██║  ██║╚██████╔╝██║ ╚═╝ ██║██║         ╚██████╗██║  ██║██║  ██║██║ ╚═╝ ██║██║");
  mvprintw(board_start_y - 1, (cols - 70) / 2, " ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═╝          ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝");

  // Subtitle
  mvprintw(board_start_y, (cols - 60) / 2, "=== 🟢 Welcome to Chomp Champs — The Ultimate Multiplayer Game! ===");

  // Board header
  mvprintw(board_start_y + 1, board_start_x, "🧩 Board (%dx%d):", state->width, state->height);

  // Draw the top border
  mvaddch(board_start_y + 2, board_start_x, ACS_ULCORNER); // Top-left corner
  for (int x = 0; x < state->width; x++) {
    mvaddstr(board_start_y + 2, board_start_x + 1 + x * 4, "───");
    mvaddch(board_start_y + 2, board_start_x + 4 + x * 4, ACS_TTEE);
  }
  mvaddch(board_start_y + 2, board_start_x + state->width * 4, ACS_URCORNER); // Top-right corner

  // Draw the board content with borders
  for (int y = 0; y < state->height; y++) {
    mvaddch(board_start_y + 3 + y, board_start_x, ACS_VLINE); // Left border
    for (int x = 0; x < state->width; x++) {
      bool printed = false;

      // Check if a player is on this cell
      for (int i = 0; i < state->num_players; i++) {
        if (state->players[i].x == x && state->players[i].y == y) {
          // Snake head with googly eyes
          attron(COLOR_PAIR(i + 1));                                         // Use the player's color
          mvaddstr(board_start_y + 3 + y, board_start_x + 1 + x * 4, "◉●◉"); // Googly eyes and circle
          attroff(COLOR_PAIR(i + 1));
          printed = true;
          break;
        }
      }

      if (!printed) {
        int value = state->board[y * state->width + x];
        if (value <= 0 && -value <= state->num_players) {
          // Snake body (same color as the head)
          int player_id = -value - 1; // Convert to 0-based index
          attron(COLOR_PAIR(player_id + 2));
          mvaddstr(board_start_y + 3 + y, board_start_x + 1 + x * 4, "■"); // Same as head but without eyes
          attroff(COLOR_PAIR(player_id + 2));
        } else if (value > 0) {
          // Display the number in white for unconsumed cells
          attron(COLOR_PAIR(17));
          mvprintw(board_start_y + 3 + y, board_start_x + 1 + x * 4, " %d ", value);
          attroff(COLOR_PAIR(17));
        } else {
          // Empty cell
          attron(COLOR_PAIR(17));
          mvaddstr(board_start_y + 3 + y, board_start_x + 1 + x * 4, " . ");
          attroff(COLOR_PAIR(17));
        }
      }
    }
    mvaddch(board_start_y + 3 + y, board_start_x + state->width * 4, ACS_VLINE); // Right border
  }

  // Draw the bottom border
  mvaddch(board_start_y + 3 + state->height, board_start_x, ACS_LLCORNER); // Bottom-left corner
  for (int x = 0; x < state->width; x++) {
    mvaddstr(board_start_y + 3 + state->height, board_start_x + 1 + x * 4, "───");
    mvaddch(board_start_y + 3 + state->height, board_start_x + 4 + x * 4, ACS_BTEE);
  }
  mvaddch(board_start_y + 3 + state->height, board_start_x + state->width * 4, ACS_LRCORNER); // Bottom-right corner

  // Players Info
  int players_info_start_y = board_start_y + 2;
  int players_info_start_x = board_start_x + state->width * 4 + 10; // Push further to the right
  mvprintw(players_info_start_y, players_info_start_x, "╔════════════════════╗");
  mvprintw(players_info_start_y + 1, players_info_start_x, "║   👥 Players Info  ║");
  mvprintw(players_info_start_y + 2, players_info_start_x, "╚════════════════════╝");
  for (int i = 0; i < state->num_players; i++) {
    const char *status_icon = state->players[i].blocked ? "🔴" : "🟢"; // Red for blocked, green for active
    const char *status_text = state->players[i].blocked ? "Blocked" : "Active";

    // Use the player's color for their name
    attron(COLOR_PAIR(i + 1));
    mvprintw(players_info_start_y + i + 3, players_info_start_x, "[%s] %s %s", state->players[i].name, status_icon, status_text);
    attroff(COLOR_PAIR(i + 1));
  }

  // Scores
  int scoreboard_start_y = players_info_start_y + state->num_players + 5; // Position below Players Info
  int scoreboard_start_x = players_info_start_x;
  mvprintw(scoreboard_start_y, scoreboard_start_x, "╔════════════════════╗");
  mvprintw(scoreboard_start_y + 1, scoreboard_start_x, "║      🏆 Scores     ║");
  mvprintw(scoreboard_start_y + 2, scoreboard_start_x, "╚════════════════════╝");
  for (int i = 0; i < state->num_players; i++) {
    attron(COLOR_PAIR(i + 1)); // Use the player's color
    mvprintw(scoreboard_start_y + i + 3, scoreboard_start_x, "%s: %d", state->players[i].name, state->players[i].score);
    attroff(COLOR_PAIR(i + 1));
  }

  refresh(); // Refresh the screen to show changes
}

void cleanup_ncurses() {
  endwin(); // Restore terminal to normal mode
}