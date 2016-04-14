/**
 * @file ejercicio6.c
 * @brief

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

/*Variables para los semaforos*/
#define SEMKEY 88888
#define SEMA_GLO 0
#define SEMA_IZQ 1
#define SEMA_DER 2
#define NUM_PROCESOS 100
#define SLEEP_MAXIMO 20

/*Variables para la memoria compartida*/
#define FILEKEY "/bin/cat"
#define KEY 888

void coche(int id, int semid, int* flujo_de_coches, int sentido) {
    int sema_dir = sentido == -1 ? SEMA_IZQ : SEMA_DER;

    sleep(rand() % SLEEP_MAXIMO);

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


void* nuevo_proceso() {
    int semid, *flujo_de_coches, key, id_zone;

    int id = getpid();

    /*Pedimos el semaforo*/
    if (Crear_Semaforo(SEMKEY, 3, &semid) == ERROR) {
        exit(0);
    }

    /*Añadir la memoria compartida*/
    key = ftok(FILEKEY, KEY);
    if (key == -1) {
        fprintf(stderr, "Error with key\n");
        exit(0);
    }

    id_zone = shmget(key, sizeof(int*), SHM_R | SHM_W);
    if (id_zone == -1) {
        fprintf(stderr, "Error with shmget 2\n");
        exit(0);
    }

    flujo_de_coches = (int*)shmat(id_zone, NULL, 0);
    if (flujo_de_coches == NULL) {
        fprintf(stderr, "Error with shmat\n");
        exit(0);
    }

    coche(id, semid, flujo_de_coches, id % 2 ? +1 : -1);

    /*Salir de la memoria compartida*/
    shmdt(flujo_de_coches);

    exit(0);
}

int main() {
    int semid, id_zone, key, *flujo_de_coches;
    int i;
    unsigned short init_arr[3] = {1, 1, 1};

    key = ftok(FILEKEY, KEY);
    if (key == -1) {
        fprintf(stderr, "Error with key\n");
        exit(0);
    }

    id_zone = shmget(key, sizeof(int*), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (id_zone == -1) {
        fprintf(stderr, "Error with shmget 1\n");
        exit(0);
    }

    if (Crear_Semaforo(SEMKEY, 3, &semid) == ERROR) {
        return 1;
    }

    Inicializar_Semaforo(semid, init_arr);

    flujo_de_coches = (int*)shmat(id_zone, NULL, 0);
    if (flujo_de_coches == NULL) {
        fprintf(stderr, "Error with shmat\n");
        exit(0);
    }

    *flujo_de_coches = 0;

    for (i = 0; i < NUM_PROCESOS; ++i) {
        if (!fork()) {
            nuevo_proceso();
        }
    }

    for (i = 0; i < NUM_PROCESOS; ++i) {
        wait(0);
    }

    if (Borrar_Semaforo(semid) == -1) {
        return 1;
    }

    shmdt(flujo_de_coches);
    shmctl(id_zone, IPC_RMID, 0);

    return 0;
}
