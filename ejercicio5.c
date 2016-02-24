#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROC 3

int main() {
    int pid;
    int i;

    printf("PRIMER PROCESO %d\n", getpid());
    pid = fork();


    for (i = 0; i < NUM_PROC; i++) {

        if (pid <0) {
            printf("Error al emplear fork\n");
            exit(EXIT_FAILURE);
        }

        else if (pid == 0) {
            printf("HIJO %d DE PADRE %d\n",getpid(), getppid());
            pid = fork();
        }
        else {
            wait(EXIT_SUCCESS);
        }

    }

    exit(EXIT_SUCCESS);
}
