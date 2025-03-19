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

// Estructuras de memoria compartida
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
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <ancho> <alto>\n", argv[0]);
        return EXIT_FAILURE;
    }

    GameState *game_state = (GameState *)connect_to_shm(SHM_GAME_STATE, sizeof(GameState));
    GameSync *game_sync = (GameSync *)connect_to_shm(SHM_GAME_SYNC, sizeof(GameSync));

    pid_t my_pid = getpid();
    int my_index = -1;
    for (unsigned int i = 0; i < game_state->player_count; i++) {
        if (game_state->players[i].pid == my_pid) {
            my_index = i;
            break;
        }
    }

    if (my_index == -1) {
        fprintf(stderr, "No se encontró el jugador en la memoria compartida.\n");
        return EXIT_FAILURE;
    }

    while (!game_state->game_over && game_state->players[my_index].can_move) {
        sem_wait(&game_sync->D);
        unsigned char move = rand() % 8; // Movimiento aleatorio (mejorable)
        sem_post(&game_sync->D);

        write(STDOUT_FILENO, &move, sizeof(move));
        usleep(50000);
    }
    return EXIT_SUCCESS;
}
