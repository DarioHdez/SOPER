/**
 * @file ejercicio2.c
 * @brief Memoria compartido sin sincronización. Condición de carrera.

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín
 * @date 2016/03/27
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

#define FILEKEY "/bin/cat"
#define KEY 666

typedef struct info_ {
    char nombre[80];
    int id;
} info_t;

info_t* shared_info;

void hijo() {
    int key, id_zone;
    info_t* my_info;

    key = ftok(FILEKEY, KEY);
    if (key == -1) {
        fprintf(stderr, "Error with key\n");
        exit(0);
    }

    id_zone = shmget(key, sizeof(info_t), IPC_EXCL | SHM_R | SHM_W);
    if (id_zone == -1) {
        fprintf(stderr, "Error with shmget\n");
        exit(0);
    }

    my_info = (info_t*)shmat(id_zone, NULL, 0);
    if (my_info == NULL) {
        fprintf(stderr, "Error with shmat\n");
        exit(0);
    }

    sleep(getpid()%20);

    /*printf("Introduzca nombre: ");
       scanf("%s", my_info->nombre);*/
    strcpy(my_info->nombre, "domo");

    int local_id = my_info->id; //NO ES ATOMICO
    sleep(1);                   //este sleep hace que la suma pueda ser interrumpida a la mitad
    my_info->id = local_id + 1; //NO ES ATOMICO

    kill(getppid(), SIGUSR1);

    shmdt(my_info);

    exit(0);
}

void manejador_SIGUSR1() {
    printf(
        "shared_info = {\n"
        "    nombre: %s\n"
        "    id: %d\n"
        "}\n",
        shared_info->nombre, shared_info->id);
}

int main(int argc, char* argv[]) {
    int key, id_zone;
    int num_procs;
    int p;

    if(argc < 2) {
        printf("Uso: %s <num-procs>\n", argv[0]);
        return -1;
    }

    num_procs = atoi(argv[1]);

    signal(SIGUSR1, manejador_SIGUSR1);

    key = ftok(FILEKEY, KEY);
    if (key == -1) {
        fprintf(stderr, "Error with key\n");
        return -1;
    }

    id_zone = shmget(key, sizeof(info_t), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (id_zone == -1) {
        fprintf(stderr, "Error with shmget\n");
        return -1;
    }

    printf("ID zone shared memory: %i\n", id_zone);

    shared_info = (info_t*)shmat(id_zone, NULL, 0);
    if (shared_info == NULL) {
        fprintf(stderr, "Error with shmat\n");
        return -1;
    }

    printf("Shared memory at: %p\n", shared_info);

    shared_info->nombre[0] = '\0';
    shared_info->id = 0;

    for(p = 0; p < num_procs; ++p) {
        if(fork() == 0) {
            hijo();
            exit(0);
        }
    }

    for(p = 0; p < num_procs; ++p) {
        wait(0);
    }

    shmdt(shared_info);
    shmctl(id_zone, IPC_RMID, 0);

    return 0;
}
