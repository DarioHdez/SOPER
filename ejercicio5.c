/**
 * @file ejercicio5.c
 * @brief Ejercicio 5: Creacion sequencial de procesos
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

#define HIJO_CREA
// #define PADRE_CREA

#ifndef HIJO_CREA
  #define PADRE_CREA /* Al menos define un modo */
#endif

#ifdef HIJO_CREA
  #undef PADRE_CREA /* HIJO_CREA tiene prioridad */
#endif

#define NUM_PROC 3

int main() {
    int pid;
    int i;

    printf("PRIMER PROCESO %d\n", getpid());


    for (i = 0; i < NUM_PROC; i++) {
        pid = fork();

        if (pid < 0) {
            printf("Error al emplear fork\n");
            exit(EXIT_FAILURE);
        }

        else if (pid == 0) {
            printf("HIJO %d DE PADRE %d\n", getpid(), getppid());
            #ifdef PADRE_CREA
            sleep(1); /*No morimos instantaneamente para que se vea que el padre sigue creando antes de esperar*/
            printf("MUERE %d\n", getpid());
            exit(EXIT_SUCCESS);
            #endif
        }
        else {
            #ifdef HIJO_CREA
            wait(EXIT_SUCCESS);
            printf("MUERE %d\n", getpid());
            exit(EXIT_SUCCESS);
            #endif
        }

    }

    #ifdef PADRE_CREA
    for (i = 0; i < NUM_PROC; i++) {
        wait(EXIT_SUCCESS);
    }
    #endif

    printf("MUERE %d\n", getpid());

    exit(EXIT_SUCCESS);
}
