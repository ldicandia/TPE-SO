#include "tads/game_logic.h"
#include "tads/shmemory.h"
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

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define GRAY "\x1b[90m" // Gris oscuro
#define ORANGE "\033[38;5;208m"

const char *colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, GRAY, ORANGE};

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

  while (!state->game_over) {
    sem_wait(&sync->A);
    print_board(state);
    sem_post(&sync->B);
  }

  printf("Todos los jugadores están bloqueados. Fin del juego.\n");
  printf("Game Over!\n");
  return 0;
}

void print_board(GameState *state) {
  system("clear");
  printf("\n=== ChompChamps ===\n");

  // Mostrar información de los jugadores
  printf("STATUS:\n");

  for (int i = 0; i < state->num_players; i++) {
    if (state->players[i].blocked) {
      printf("Jugador %s está bloqueado.\n", state->players[i].name);
    } else {
      printf("Jugador %s - Pos: (%d, %d), Score: %d, Bloqueado: %d\n", state->players[i].name, state->players[i].x, state->players[i].y, state->players[i].score, state->players[i].blocked);
    }
  }

  // Copiar el tablero para poder sobreescribirlo con jugadores
  char display_board[state->height][state->width];
  for (int y = 0; y < state->height; y++) {
    for (int x = 0; x < state->width; x++) {
      display_board[y][x] = '0' + state->board[y * state->width + x];
    }
  }

  // Imprimir el tablero modificado
  for (int y = 0; y < state->height; y++) {
    for (int x = 0; x < state->width; x++) {
      char elem = display_board[y][x];
      if (elem <= '0') {
        printf("%s %s \033[0m", colors['0' - elem], "\u25A0");
      } else {
        printf(" %c ", elem);
      }
    }
    printf("\n");
  }

  printf("\nPlayers:\n");
  for (int i = 0; i < state->num_players; i++) {
    printf("%s - Score: %d\n", state->players[i].name, state->players[i].score);
  }
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