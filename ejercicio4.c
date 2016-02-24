/**
 * @file ejercicio4.c
 * @brief Ejercicio 4:
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-02-09
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROC 3

int main() {
    int pid;
    int i;

    for (i = 0; i < NUM_PROC; i++) {
        pid = fork();

        if(pid <0) {
            printf("Error al emplear fork\n");
            exit(EXIT_FAILURE);
        } else if (pid ==0) {
            printf("HIJO %d\n ", getpid());
        } else {
            printf("PADRE %d\n", getpid());
            wait(EXIT_SUCCESS);
        }
    }

    exit(EXIT_SUCCESS);
}
