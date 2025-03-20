//fork + shm + sem
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

void *createSHM(char *name, size_t size){
  int fd;
  fd = shm_open(name, O_RDWR | O_CREAT, 0666);
  if (fd == -1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(fd, size) == -1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }
  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  return ptr;
}

int main(void){
  int *ptr = (int*)createSHM("/my_shm", sizeof(int));
  *ptr = 5;
  pid_t pid; 

  sem_t *print_needed = (sem_t *)createSHM("/print_needed", sizeof(sem_t));
  if(sem_init(print_needed, 1, 0) == -1){
    perror("sem_init");
    exit(EXIT_FAILURE);
  }
  sem_t *print_done = (sem_t *)createSHM("/print_done", sizeof(sem_t));
  if(sem_init(print_done, 1, 0) == -1){
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  pid = fork();
  switch(pid){
    case -1:
      perror("fork");
      exit(EXIT_FAILURE);
    case 0:
      //child
      for(int i = 0; i < 3 ;i++){
        sem_wait(print_needed);
        printf("Child: %d\n", *ptr);
        sem_post(print_done);
      }
      exit(EXIT_SUCCESS);
    default:
      //parent
      for(int i = 0 ; i < 3 ; i++){
        *ptr = i;
        printf("Parent: %d\n", *ptr);
        sem_post(print_needed);
        sem_wait(print_done);
      }
      exit(EXIT_SUCCESS);
  }
}