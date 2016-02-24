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

    if (pid <0) {
        printf("Error al emplear fork\n");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        printf("Escribe tu nombre: \n");
        scanf("%s", name);
    }

    free(name);

    wait(EXIT_SUCCESS);
    exit(EXIT_SUCCESS);
}
