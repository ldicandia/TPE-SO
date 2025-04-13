// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "view.h"

const char *colors[]	  = {RED,  GREEN, YELLOW, BLUE, MAGENTA,
							 CYAN, GRAY,  ORANGE, WHITE};
const char *dark_colors[] = {DARK_RED,	DARK_GREEN,	  DARK_YELLOW,
							 DARK_BLUE, DARK_MAGENTA, DARK_CYAN,
							 DARK_GRAY, DARK_ORANGE,  DARK_WHITE};

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int width  = atoi(argv[1]);
	int height = atoi(argv[2]);

	size_t game_state_size =
		get_game_state_size() + width * height * sizeof(int);

	GameState *state = attach_shared_memory(SHM_GAME_STATE, game_state_size,
											O_RDONLY, PROT_READ);
	GameSync *sync	 = attach_shared_memory(SHM_GAME_SYNC, get_game_sync_size(),
											O_RDWR, PROT_READ | PROT_WRITE);

	while (!is_game_over(state)) {
		semaphore_pre_print(sync);
		print_board(state);
		semaphore_post_print(sync);
	}
	semaphore_game_over(sync);

	system("clear");
	// GAME OVER
	printf("\033[38;5;201m ██████╗  █████╗ ███╗   ███╗███████╗     ██████╗ ██╗ "
		   "  ██╗███████╗██████╗    \n");
	printf("██╔════╝ ██╔══██╗████╗ ████║██╔════╝    ██╔═══██╗██║   "
		   "██║██╔════╝██╔══██║   \n");
	printf("██║  ███╗███████║██╔████╔██║█████╗      ██║   ██║██║   ██║█████╗  "
		   "█████╔═╝   \n");
	printf("██║   ██║██╔══██║██║╚██╔╝██║██╔══╝      ██║   ██║███║ ███║██╔══╝  "
		   "██╔███╗    \n");
	printf("╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗    ╚██████╔╝ ╚████╔╝ "
		   "███████╗██║ ███╗   \n");
	printf(" ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝     ╚═════╝   ╚═══╝  "
		   "╚══════╝╚═╝ ╚══╝   \033[0m\n");

	printf("\n🏆 \033[1mPuntajes:\033[0m\n");

	int indices[MAX_PLAYERS - 1];
	for (int i = 0; i < get_num_players(state); i++)
		indices[i] = i;

	// Ordenar por puntaje
	for (int i = 0; i < get_num_players(state) - 1; i++) {
		for (int j = 0; j < get_num_players(state) - i - 1; j++) {
			if (get_player_score(state, indices[j]) <
				get_player_score(state, indices[j + 1])) {
				int temp	   = indices[j];
				indices[j]	   = indices[j + 1];
				indices[j + 1] = temp;
			}
		}
	}

	for (int i = 0; i < get_num_players(state); i++) {
		int idx = indices[i];
		if (i == 0) {
			printf(" %s[%s]\033[0m: %d \033[38;5;220mGANADOR 🏆\033[0m\n",
				   colors[idx], get_player_name(state, idx),
				   get_player_score(state, idx));
		}
		else {
			printf(" %s[%s]\033[0m: %d\n", colors[idx],
				   get_player_name(state, idx), get_player_score(state, idx));
		}
	}

	printf("\n\033[1m=========================================================="
		   "===========================\033[0m\n\n");
	return 0;
}

void print_board(GameState *state) {
	system("clear");

	// CHOMP CHAMPS
	printf("\033[38;5;201m ██████╗██╗  ██╗ ██████╗ ███╗   ███╗██████╗      "
		   "██████╗██╗  ██╗ █████╗ ███╗   ███╗██████╗ ██████╗      \n");
	printf("██╔════╝██║  ██║██╔═══██╗████╗ ████║██╔══██╗    ██╔════╝██║  "
		   "██║██╔══██╗████╗ ████║██╔══██╗██╔═══╝      \n");
	printf("██║     ███████║██║   ██║██╔████╔██║██████╔╝    ██║     "
		   "███████║███████║██╔████╔██║██████╔╝██████╗      \n");
	printf("██║     ██╔══██║██║   ██║██╔╝██╔╝██║██╔═══╝     ██║     "
		   "██╔══██║██╔══██║██╔╝██╔╝██║██╔═══╝     ██║      \n");
	printf("╚██████╗██║  ██║╚██████╔╝██║ ╚═╝ ██║██║         ╚██████╗██║  "
		   "██║██║  ██║██║ ╚═╝ ██║██║     ██████║      \n");
	printf(" ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝╚═╝          ╚═════╝╚═╝  "
		   "╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝     ╚═════╝      \033[0m\n");

	printf("👥 \033[1mPlayers Info:\033[0m\n");
	for (int i = 0; i < get_num_players(state); i++) {
		const char *status =
			is_player_blocked(state, i) ? "🚫 Bloqueado" : "✅ Activo";
		printf("%s[%s]\033[0m %s - Pos: (%d,%d)\n", colors[i],
			   get_player_name(state, i), status, get_player_x(state, i),
			   get_player_y(state, i));
	}

	printf("\n🧩 \033[1mBoard (%dx%d):\033[0m\n\n", get_width(state),
		   get_height(state));

	printf("    ");
	for (int x = 0; x < get_width(state); x++) {
		printf(" %2d ", x);
	}
	printf("\n");

	printf("   ");
	for (int x = 0; x < get_width(state); x++) {
		printf("────");
	}
	printf("─\n");

	for (int y = 0; y < get_height(state); y++) {
		printf(" %2d│", y);
		for (int x = 0; x < get_width(state); x++) {
			bool printed = false;
			for (int i = 0; i < get_num_players(state); i++) {
				if (get_player_x(state, i) == x &&
					get_player_y(state, i) == y) {
					if (is_player_blocked(state, i)) {
						printf("%s ✝  \033[0m", dark_colors[i]);
					}
					else {
						printf("%s ■  \033[0m", dark_colors[i]);
					}
					printed = true;
					break;
				}
			}

			if (!printed) {
				int value = get_state_value(state, x, y);

				if (value <= 0 && -value < MAX_PLAYERS) {
					printf("%s ■  \033[0m",
						   colors[-value]); // Casilla coloreada
				}
				else if (value > 0) {
					printf(" %2d ", value);
				}
				else {
					printf(" .. ");
				}
			}
		}
		printf("│\n");
	}

	printf("    ");
	for (int x = 0; x < get_width(state); x++) {
		printf("────");
	}
	printf("─\n");

	printf("\n🏆 \033[1mPuntajes:\033[0m\n");

	int indices[8];
	for (int i = 0; i < get_num_players(state); i++)
		indices[i] = i;

	// Ordenar por puntaje
	for (int i = 0; i < get_num_players(state) - 1; i++) {
		for (int j = 0; j < get_num_players(state) - i - 1; j++) {
			if (get_player_score(state, indices[j]) <
				get_player_score(state, indices[j + 1])) {
				if (get_player_pid(state, indices[j]) <
					get_player_pid(state, indices[j + 1])) {
					int temp	   = indices[j];
					indices[j]	   = indices[j + 1];
					indices[j + 1] = temp;
				}
			}
		}
	}

	for (int i = 0; i < get_num_players(state); i++) {
		int idx = indices[i];
		printf(" %s[%s]\033[0m: %d\n", colors[idx], get_player_name(state, idx),
			   get_player_score(state, idx));
	}

	// Leyenda
	printf("\n🗒️  \033[1mLeyenda:\033[0m\n");
	printf(" ■  Casilla de color del jugador\n");
	printf(" ✝  Jugador bloqueado\n");
	printf(" ■  Jugador activo\n");

	printf("\n\033[1m=========================================================="
		   "=========================================\033[0m\n\n");
}
