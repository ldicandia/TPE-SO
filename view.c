#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

// Estructuras basadas en el enunciado
typedef struct {
    char name[16];
    unsigned int score;
    unsigned int invalid_moves;
    unsigned int valid_moves;
    unsigned short x, y;
    pid_t pid;
    bool can_move;
} Player;

typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned int player_count;
    Player players[9];
    bool game_over;
    int board[]; // El tablero se almacena al final
} GameState;

typedef struct {
    sem_t print_needed;
    sem_t print_done;
} GameSync;

// Función para conectar la vista a una memoria compartida
void *connect_to_shm(char *name, size_t size) {
    int fd = shm_open(name, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    void *ptr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

// Función para imprimir el tablero
void print_board(GameState *state) {
    //system("clear"); // Limpiar pantalla (opcional)

    printf("=== ChompChamps ===\n");
    printf("Tablero: %dx%d\n", state->width, state->height);

    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            int cell = state->board[y * state->width + x];
            if (cell == -1) {
                printf(". "); // Celda vacía
            } else {
                printf("%d ", cell); // Celda con recompensa o jugador
            }
        }
        printf("\n");
    }

    printf("\nJugadores:\n");
    for (unsigned int i = 0; i < state->player_count; i++) {
        printf("%s - Score: %d, Posición: (%d, %d)\n",
               state->players[i].name,
               state->players[i].score,
               state->players[i].x,
               state->players[i].y);
    }
}

int main() {
    // Conectar a las memorias compartidas
    GameState *game_state = (GameState *)connect_to_shm(SHM_GAME_STATE, sizeof(GameState));
    GameSync *game_sync = (GameSync *)connect_to_shm(SHM_GAME_SYNC, sizeof(GameSync));

    while (!game_state->game_over) {
        sem_wait(&game_sync->print_needed); // Esperar señal del máster
        print_board(game_state);            // Imprimir estado
        sem_post(&game_sync->print_done);   // Avisar al máster que terminó
    }

    printf("Juego terminado.\n");
    return 0;
}
