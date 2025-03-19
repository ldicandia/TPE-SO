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
    int board[];
} GameState;

typedef struct {
    sem_t A, B, C, D, E;
    unsigned int F;
} GameSync;

void *connect_to_shm(char *name, size_t size) {
    int fd = shm_open(name, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, size) == -1){
      perror("ftruncate");
      exit(EXIT_FAILURE);
    }

    void *ptr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void print_board(GameState *state) {
    printf("Esperando a que comience el juego...\n");
    //system("clear");
    printf("=== ChompChamps ===\n");
    printf("Tablero: %dx%d\n", state->width, state->height);
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            int cell = state->board[y * state->width + x];
            if (cell == -1) {
                printf(". ");
            } else {
                printf("%d ", cell);
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
    GameState *game_state = (GameState *)connect_to_shm(SHM_GAME_STATE, sizeof(GameState));
    GameSync *game_sync = (GameSync *)connect_to_shm(SHM_GAME_SYNC, sizeof(GameSync));

    while (!game_state->game_over) {
        //sem_wait(&game_sync->A);
        print_board(game_state);
        sem_post(&game_sync->B);
    }
    printf("Juego terminado.\n");
    return 0;
}
