/**
 * @file ejercicio10.c
 * @brief Ejercicio 10: Lectura de un fichero con procesos, señales y alarmas 
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-3-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define FILENAME "salida.txt"

char* tokens[] = {
    "EL", "PROCESO", "A", "ESCRIBE", "EN", "UN", "FICHERO", "HASTA", "QUE", "LEE", "LA", "CADENA", "FIN"
};

void procesoA() {
    FILE* file = fopen(FILENAME, "a");

    if(file == NULL) {
        exit(1);
    }

    srand(time(0));

    for(;;) {
        int w = rand()%13;
        char* word = tokens[w];
        
        fprintf(file, "%s\n", word);

        if(strcmp(word, "FIN") == 0) {
            fclose(file);
            exit(0);
        }
    }
}

FILE* file = NULL;
int p = 0;

void handler_alarma() {
    char buf[100];

    char* ret = fgets(buf, sizeof(buf), file);

    printf("Leyendo %s", ret == NULL ? "NULL\n" : buf);

    if(ret == NULL || strcmp(buf, "FIN\n") == 0) {
        int pid;

        pid = fork();

        if(++p >= 3) {
            fclose(file);
            exit(0);
        }

        if(pid == 0) {
            procesoA();
            exit(0);
        }
    }
}

void procesoB() {
    int pid;

    file = fopen(FILENAME, "w+");

    if(file == NULL) {
        exit(1);
    }

    pid = fork();

    if(pid == 0) {
        procesoA();
    } else {
        sleep(1);

        signal(SIGALRM, handler_alarma);

        for(;;) {
            alarm(1);
            pause();
        }
    }
}

int main() {
    procesoB();

    return 0;
}

