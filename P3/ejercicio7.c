/**
 * @file ejercicio7.c
 * @brief Implementacion del puente con hilos
 *
 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín
 * @date 2016/04/06
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "semaforos.h"

int flujo_de_coches;
int semid;

#define SEMKEY 66666
#define SEMA_GLO 0
#define SEMA_IZQ 1
#define SEMA_DER 2
#define NUM_HILOS 10

void coche(int id, int semid, int* flujo_de_coches, int sentido) {
    int sema_dir = sentido == -1 ? SEMA_IZQ : SEMA_DER;

    sleep(rand() % 10);

    Down_Semaforo(semid, sema_dir, 1);

    printf("%d: Entrando de puente con sentido %d\n", id, sentido);

    if ((*flujo_de_coches)*sentido <= 0) { // Si no estan pasando o pasan en el otro sentido
        Down_Semaforo(semid, SEMA_GLO, 1); // espero a establecer mi sentido
    }

    *flujo_de_coches += sentido;

    Up_Semaforo(semid, sema_dir, 1);

    printf("%d: En el puente con sentido %d\nflujo_de_coches = %d\n", id, sentido, *flujo_de_coches);
    sleep(1);

    Down_Semaforo(semid, sema_dir, 1);

    *flujo_de_coches -= sentido;
    printf("%d: Saliendo de puente con sentido %d\n", id, sentido);

    if (*flujo_de_coches == 0) {
        Up_Semaforo(semid, SEMA_GLO, 1);
    }

    Up_Semaforo(semid, sema_dir, 1);
}

void* nuevo_hilo(void* arg) {
    int id = *((int*)arg);

    coche(id, semid, &flujo_de_coches, id % 2 ? +1 : -1);

    pthread_exit(NULL);
}

int main() {
    int i;
    unsigned short init_arr[3] = {1, 1, 1};

    if (Crear_Semaforo(SEMKEY, 3, &semid) == ERROR) {
        fprintf(stderr, "Error creando semaforo\n");
        return 1;
    }

    Inicializar_Semaforo(semid, init_arr);

    flujo_de_coches = 0;

    pthread_t th[NUM_HILOS];
    int th_ids[NUM_HILOS];

    for (i = 0; i < NUM_HILOS; ++i) {
        th_ids[i] = i;
        if (pthread_create(&th[i], NULL, nuevo_hilo, (void*)&th_ids[i])) {
            fprintf(stderr, "Error creando hilo %d\n", i);
            return 1;
        }
    }

    for (i = 0; i < NUM_HILOS; ++i) {
        if (pthread_join(th[i], NULL)) {
            fprintf(stderr, "Error esperando hilo %d\n", i);
            return 1;
        }
    }

    if (Borrar_Semaforo(semid) == -1) {
        return 1;
    }

    return 0;
}
