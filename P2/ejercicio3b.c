/**
 * @file ejercicio3b.c
 * @brief Ejercicio 3b: Prueba el tiempo de 100 hilos escribiendo un numero (Creacion, escritura y finalizacion)
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-3-04
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 100

void* hilo(void *arg) {
    long id = (long)arg;

    srand(id);

    printf("%d\t", rand());
    
    pthread_exit(0);
}

int main() {
    clock_t start, end;
    double dt;
    long i;
    pthread_t th[NUM_THREADS];

    start = clock();

    for(i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&th[i], NULL, hilo, (void*)i);
    }

    for(i = 0; i < NUM_THREADS; ++i) {
        pthread_join(th[i], NULL);
    }

    end = clock();

    dt = ((double)end - start)/CLOCKS_PER_SEC;

    printf("\n\nSe ha tardado %f segundos\n", dt);

    return 0;
}