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
        
        fprintf(file, "%s\nLOL\n", word);

        if(strcmp(word, "FIN") == 0) {
            fclose(file);
            exit(0);
        }
    }
}

void procesoB() {
    int p;
    FILE* file;
    char buf[100];

    file = fopen(FILENAME, "w+");

    if(file == NULL) {
        exit(1);
    }

    for(p = 0; p < 3; ++p) {
        int pid;

        pid = fork();

        if(pid == 0) {
            procesoA();
        } else {
            sleep(1);

            for(;;) {
                char* ret = fgets(buf, sizeof(buf), file);

                if(ret == NULL) {
                    printf("ret == NULL\n");
                    break;
                }

                printf("Leyendo %s", buf);

                if(strcmp(buf, "FIN") == 0) {
                    break;
                }

                sleep(1);
            }
        }
    }

    fclose(file);
}

int main() {
    procesoB();

    return 0;
}

