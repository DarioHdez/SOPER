/**
 * @file ejercicio6.c
 * @brief Ejercicio 6: Memoria y procesos: memoria dinamica copiada
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

    char *name = (char*)malloc(20*sizeof(name[0]));

    pid = fork();

    if (pid < 0) {
        printf("Error al emplear fork\n");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        printf("Escribe tu nombre: \n");
        scanf("%s", name);
    }

    free(name); /* Liberamos memoria en los 2 procesos */

    wait(EXIT_SUCCESS);
    exit(EXIT_SUCCESS);
}
