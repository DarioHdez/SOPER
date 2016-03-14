/**
 * @file ejercicio6.c
 * @brief Ejercicio 6: Introducción a señales: Kill
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-3-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

int hijo() {
    int pid = getpid();

    for(;;) {
        printf("Soy el proceso hijo con PID: %d\n", pid);
        sleep(5);    
    }

    return 0;
}

int main() {
    int pid;

    pid = fork();

    if(pid == 0) {
        return hijo();
    } else {
        sleep(30);
        kill(pid, SIGTERM);
    }

    return 0;
}
