#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    char *name = argv[0];

    char *prog_args[] = { "du", "--apparent-size", "-BK", name, NULL };
    execvp("du", prog_args);

    perror("fallo en exec");
    exit(EXIT_FAILURE);
}
