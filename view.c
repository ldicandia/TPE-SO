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

#include "tads/game_logic.h"
#include "tads/shmemory.h"

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

const char *colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, GRAY, ORANGE, WHITE};
const char *dark_colors[] = {DARK_RED, DARK_GREEN, DARK_YELLOW, DARK_BLUE, DARK_MAGENTA, DARK_CYAN, DARK_GRAY, DARK_ORANGE, DARK_WHITE};

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

void print_board(GameState *state);

void check_players_blocked(GameState *state);

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

  while (true) {
    if (state->game_over) {
      sem_post(&sync->sem_master_ready);
      break;
    }
    sem_wait(&sync->sem_view_ready);
    print_board(state);
    sem_post(&sync->sem_master_ready);
  }

  printf(" ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗   ██╗███████╗██████╗    \n");
  printf("██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   ██║██╔════╝██╔══██╗   \n");
  printf("██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  █████╔╝    \n");
  printf("██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║██║   ██║██╔══╝  ██╔███╔═╝  \n");
  printf("╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝╚██████╔╝███████╗██║ ███║   \n");
  printf(" ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝  ╚═════╝ ╚══════╝╚═╝ ╚══╝   \n");

  return 0;
}

void print_board(GameState *state) {
  system("clear");
  // printf("\n\033[1m=== 🟢 ChompChamps — Game Status ===\033[0m\n\n");

  printf(" ██████╗██╗  ██╗ ██████╗ ███╗   ███╗██████╗      ██████╗██╗  ██╗ █████╗ ███╗   ███╗██████╗ ██████╗      \n");
  printf("██╔════╝██║  ██║██╔═══██╗████╗ ████║██╔══██╗    ██╔════╝██║  ██║██╔══██╗████╗ ████║██╔══██╗██╔═══╝      \n");
  printf("██║     ███████║██║   ██║██╔████╔██║██████╔╝    ██║     ███████║███████║██╔████╔██║██████╔╝██████╗      \n");
  printf("██║     ██╔══██║██║   ██║██╔╝██╔╝██║██╔═══╝     ██║     ██╔══██║██╔══██║██╔╝██╔╝██║██╔═══╝     ██║      \n");
  printf("╚██████╗██║  ██║╚██████╔╝██║ ╚═╝ ██║██║         ╚██████╗██║  ██║██║  ██║██║ ╚═╝ ██║██║     ██████║      \n");
  printf(" ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═╝          ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝     ╚═════╝      \n");

  printf("👥 \033[1mPlayers Info:\033[0m\n");
  for (int i = 0; i < state->num_players; i++) {
    const char *status = state->players[i].blocked ? "🚫 Bloqueado" : "✅ Activo";
    printf("%s[%s]\033[0m %s - Pos: (%d,%d)\n", colors[i], state->players[i].name, status, state->players[i].x, state->players[i].y);
  }

  printf("\n🧩 \033[1mBoard (%dx%d):\033[0m\n\n", state->width, state->height);

  printf("    ");
  for (int x = 0; x < state->width; x++) {
    printf(" %2d ", x);
  }
  printf("\n");

  printf("   ");
  for (int x = 0; x < state->width; x++) {
    printf("────");
  }
  printf("─\n");

  for (int y = 0; y < state->height; y++) {
    printf(" %2d│", y);
    for (int x = 0; x < state->width; x++) {
      bool printed = false;
      for (int i = 0; i < state->num_players; i++) {
        if (state->players[i].x == x && state->players[i].y == y) {
          if (state->players[i].blocked) {
            printf("%s ✝  \033[0m", dark_colors[i]);
          } else {
            printf("%s ■  \033[0m", dark_colors[i]);
          }
          printed = true;
          break;
        }
      }

      if (!printed) {
        int value = state->board[y * state->width + x];
        if (value <= 0 && -value < 9) {
          printf("%s ■  \033[0m", colors[-value]); // Casilla coloreada
        } else if (value > 0) {
          printf(" %2d ", value);
        } else {
          printf(" .. ");
        }
      }
    }
    printf("│\n");
  }

  printf("    ");
  for (int x = 0; x < state->width; x++) {
    printf("────");
  }
  printf("─\n");

  printf("\n🏆 \033[1mPuntajes:\033[0m\n");

  int indices[8];
  for (int i = 0; i < state->num_players; i++)
    indices[i] = i;

  // Ordenar por puntaje
  for (int i = 0; i < state->num_players - 1; i++) {
    for (int j = 0; j < state->num_players - i - 1; j++) {
      if (state->players[indices[j]].score < state->players[indices[j + 1]].score) {
        int temp = indices[j];
        indices[j] = indices[j + 1];
        indices[j + 1] = temp;
      }
    }
  }

  for (int i = 0; i < state->num_players; i++) {
    int idx = indices[i];
    printf(" %s[%s]\033[0m: %d\n", colors[idx], state->players[idx].name, state->players[idx].score);
  }

  // Leyenda
  printf("\n🗒️  \033[1mLeyenda:\033[0m\n");
  printf(" ■  Casilla de color del jugador\n");
  printf(" ✝  Jugador bloqueado\n");
  printf(" ■  Jugador activo\n");

  printf("\n\033[1m===================================================================================================\033[0m\n\n");
}

void check_players_blocked(GameState *state) {
  bool all_blocked = true;
  for (int i = 0; i < state->num_players; i++) {
    if (!state->players[i].blocked) {
      all_blocked = false;
      break;
    }
  }

  if (all_blocked) {
    state->game_over = true;
  }
}