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

#include "tads/game_logic.h"
#include "tads/shmemory.h"

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

void print_board(GameState *state);
void init_colors();

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int width = atoi(argv[1]);
  int height = atoi(argv[2]);

  size_t game_state_size = sizeof(GameState) + width * height * sizeof(int);

  GameState *state = attach_shared_memory(SHM_GAME_STATE, game_state_size, O_RDONLY, PROT_READ);
  GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync), O_RDWR, PROT_READ | PROT_WRITE);

  initscr();
  noecho();
  cbreak();
  curs_set(FALSE);
  start_color();
  init_colors();

  while (true) {
    if (state->game_over) {
      sem_post(&sync->sem_master_ready);
      break;
    }
    sem_wait(&sync->sem_view_ready);
    clear();
    print_board(state);
    refresh();
    sem_post(&sync->sem_master_ready);
    usleep(200000); // Evita parpadeo excesivo
  }

  endwin();
  printf("Todos los jugadores están bloqueados. Fin del juego.\n");
  printf("Game Over!\n");
  return 0;
}

void print_board(GameState *state) {
  int max_y, max_x;
  getmaxyx(stdscr, max_y, max_x);
  int cell_width = 5;
  int border_width = 2;
  int total_width = state->width * cell_width + border_width * 2;
  int offset_x = (max_x - total_width) / 2;
  int offset_y = (max_y - (state->height + 10)) / 2;

  if (offset_x < 0)
    offset_x = 0;
  if (offset_y < 0)
    offset_y = 0;

  mvprintw(0, (max_x - 28) / 2, "=== ChompChamps - Game Status ===");

  mvprintw(offset_y++, offset_x, "Players:");
  for (int i = 0; i < state->num_players; i++) {
    const char *status = state->players[i].blocked ? "Blocked" : "Active";
    attron(COLOR_PAIR(i + 1));
    mvprintw(offset_y++, offset_x, "  %s - %s - Pos: (%d,%d) - Score: %d", state->players[i].name, status, state->players[i].x, state->players[i].y, state->players[i].score);
    attroff(COLOR_PAIR(i + 1));
  }

  offset_y += 1;

  mvprintw(offset_y++, offset_x, "|");
  for (int i = 0; i < state->width * cell_width - 2; i++) {
    printw("=");
  }
  printw("|");

  mvprintw(offset_y++, offset_x, "| ");
  for (int x = 0; x < state->width; x++) {
    mvprintw(offset_y - 1, offset_x + 2 + x * cell_width, "%2d", x);
  }
  printw(" |");

  for (int y = 0; y < state->height; y++) {
    mvprintw(offset_y++, offset_x, "|");

    for (int x = 0; x < state->width; x++) {
      bool player_here = false;

      for (int i = 0; i < state->num_players; i++) {
        if (state->players[i].x == x && state->players[i].y == y) {
          attron(COLOR_PAIR(i + 1));
          mvprintw(offset_y - 1, offset_x + 2 + x * cell_width, " O ");
          attroff(COLOR_PAIR(i + 1));
          player_here = true;
          break;
        }
      }

      if (!player_here) {
        int value = state->board[y * state->width + x];
        if (value <= 0 && -value < 8) {
          attron(COLOR_PAIR(-value + 1));
          mvprintw(offset_y - 1, offset_x + 2 + x * cell_width, " # ");
          attroff(COLOR_PAIR(-value + 1));
        } else if (value > 0) {
          mvprintw(offset_y - 1, offset_x + 2 + x * cell_width, "%2d ", value);
        } else {
          mvprintw(offset_y - 1, offset_x + 2 + x * cell_width, " . ");
        }
      }
    }

    printw("|");
  }

  mvprintw(offset_y++, offset_x, "|");
  for (int i = 0; i < state->width * cell_width - 2; i++) {
    printw("=");
  }
  printw("|");

  offset_y += 1;
  mvprintw(offset_y++, offset_x, "----------------- Scoreboard -----------------");
  for (int i = 0; i < state->num_players; i++) {
    attron(COLOR_PAIR(i + 1));
    mvprintw(offset_y++, offset_x, "Player %d: %s - Score: %2d", i + 1, state->players[i].name, state->players[i].score);
    attroff(COLOR_PAIR(i + 1));
  }
  mvprintw(offset_y++, offset_x, "---------------------------------------------");
}

void init_colors() {
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_CYAN, COLOR_BLACK);
  init_pair(7, COLOR_WHITE, COLOR_BLACK);
  init_pair(8, COLOR_BLACK, COLOR_YELLOW);

  init_pair(9, COLOR_BLACK, COLOR_RED);
  init_pair(10, COLOR_BLACK, COLOR_GREEN);
  init_pair(11, COLOR_BLACK, COLOR_YELLOW);
  init_pair(12, COLOR_BLACK, COLOR_BLUE);
  init_pair(13, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(14, COLOR_BLACK, COLOR_CYAN);
  init_pair(15, COLOR_BLACK, COLOR_WHITE);
  init_pair(16, COLOR_BLACK, COLOR_YELLOW);
}