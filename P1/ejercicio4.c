/**
 * @file ejercicio4.c
 * @brief Ejercicio 4: Arbol de procesos y wait
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

/*#define WAIT*/ /*Esperar una vez*/
#define WAIT_ALL /*Esperar tantas veces como procesos hijo*/
/*En caso de comentar las dos no se espera*/

#ifdef WAIT_ALL /*Es caso de haber descomentado las dos WAIT_ALL tiene prioridad*/
  #undef WAIT
#endif

#define NUM_PROC 3

int main() {
    int pid;
    int i;
    #ifdef WAIT_ALL
    int p = 0;
    #endif

    printf("PID = %d\n", getpid());
    sleep(6);

    for (i = 0; i < NUM_PROC; i++) {
        pid = fork();

        if(pid < 0) {
            printf("Error al emplear fork\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            printf("HIJO %d CON PADRE %d\n", getpid(), getppid());
            #ifdef WAIT_ALL
            p = i;
            #endif
            sleep(3);
        } else {
            printf("PADRE %d\n", getpid());
        }
    }

    #ifdef WAIT
    wait(EXIT_SUCCESS);
    #endif

    #ifdef WAIT_ALL
    for (i = p; i < NUM_PROC; i++) {
        wait(EXIT_SUCCESS);
    }
    #endif

    printf("MUERE %d\n", getpid());

    sleep(1);

    exit(EXIT_SUCCESS);
}
