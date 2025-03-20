#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define SHM_GAME_STATE "/game_state"
#define SHM_GAME_SYNC "/game_sync"

typedef struct {
    char name[16];
    unsigned int score;
    unsigned int invalid_moves;
    unsigned int valid_moves;
    unsigned short x, y;
    pid_t pid;
    bool active;
} Player;

typedef struct {
    unsigned short width;
    unsigned short height;
    unsigned int num_players;
    Player players[9];
    bool game_over;
    int board[];
} GameState;

typedef struct {
    sem_t A;
    sem_t B;
} GameSync;

void *attach_shared_memory(const char *name, size_t size) {
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

void print_board(GameState *state) {
    system("clear");
    printf("\n=== ChompChamps ===\n");
    
    // Mostrar información de los jugadores

    printf("\nDEBUG: Estado de los jugadores\n");
    
    for (int i = 0; i < state->num_players; i++) {
        printf("Jugador %s - Pos: (%d, %d), Score: %d, Activo: %d\n",
               state->players[i].name, state->players[i].x, state->players[i].y,
               state->players[i].score, state->players[i].active);
    }
    
    // Copiar el tablero para poder sobreescribirlo con jugadores
    char display_board[state->height][state->width];
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            display_board[y][x] = '0' + state->board[y * state->width + x];
        }
    }
        

    // Colocar jugadores en el tablero
    // for (int i = 0; i < state->num_players; i++) {
    //     state->players[i].active = 0;
    //     if (state->players[i].active) {
    //         unsigned short px = state->players[i].x;
    //         unsigned short py = state->players[i].y;
            
    //         if (px < state->width && py < state->height) {
    //             display_board[py][px] = '/';
    //         }
    //     }
    // }


    // Imprimir el tablero modificado
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            char elem = display_board[y][x];
            if( elem == '0'){
                printf("\033[31m %c \033[0m", elem);
            }
            else{
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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <width> <height>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    GameState *state = attach_shared_memory(SHM_GAME_STATE, sizeof(GameState));
    GameSync *sync = attach_shared_memory(SHM_GAME_SYNC, sizeof(GameSync));
    
    while (!state->game_over) {
        sem_wait(&sync->B);
        //sleep(1);
        print_board(state);
        sem_post(&sync->A);
    }
    
    printf("Game Over!\n");
    exit(1);
    return 0;
}