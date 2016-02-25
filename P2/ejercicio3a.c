#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_PROCS 100

void hijo() {
    srand(getpid());

    printf("%d\t", rand());
    exit(0);
}

int main() {
    int pid;
    int i;
    clock_t start, end;
    double dt;

    start = clock();

    for(i = 0; i < NUM_PROCS; ++i) {
        pid = fork();

        if(pid == 0) {
            hijo();
        }
    }

    for(i = 0; i < NUM_PROCS; ++i) {
        int status;

        wait(&status);
    }

    end = clock();

    dt = ((double)end - start)/CLOCKS_PER_SEC;

    printf("\n\nSe ha tardado %f segundos\n", dt);

    return 0;
}