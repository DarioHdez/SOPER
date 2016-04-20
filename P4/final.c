#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <pthread.h>

#include "semaforos.h"

#define NUM_AULAS 2
#define NUM_PROFS_POR_AULA 2

#define CHECK(f) (check_err(#f, (f)))

#define FILEKEY "/bin/cat"
#define KEY_SEM 24567
#define BASEKEY_SHM 34567
#define BASEKEY_MSQ 44567
#define MSQ_PERMS 0600

typedef pid_t asiento_t;

typedef struct {
    int num_sem;
    pthread_t hilo;
    int esperando;
    int aula;
} alumno_state_t;

int check_err(const char *fname, int val);
void* check_mem(void* mem);

void* alumno(void *arg);
int profesor(int aula, key_t clave_aula, key_t clave_msq);

alumno_state_t *estado_alumno;

int semid;

int main() {
    unsigned int tamanio_aula[NUM_AULAS];

    key_t clave_aula[NUM_AULAS];
    int shm_aula[NUM_AULAS];
    void* mem_aula[NUM_AULAS];

    key_t clave_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];
    int msq_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];

    key_t clave_sem = CHECK(ftok(FILEKEY, KEY_SEM));

    unsigned int numero_alumnos;
    unsigned int i, j;

    for (i = 0; i < NUM_AULAS; ++i) {
        printf("Introduzca el numero de asientos en aula %d: ", i + 1);
        scanf("%u", &tamanio_aula[i]);
    }

    printf("Introduzca el numero de alumnos: ");
    scanf("%u", &numero_alumnos);

    estado_alumno = (alumno_state_t*)check_mem(malloc(numero_alumnos*sizeof(alumno_state_t)));

    Crear_Semaforo(clave_sem, numero_alumnos, &semid);
    unsigned short* inicializacion = (unsigned short*)check_mem(alloca(numero_alumnos*sizeof(unsigned short)));
    memset(inicializacion, 0, numero_alumnos*sizeof(unsigned short));
    Inicializar_Semaforo(semid, inicializacion);

    int pid;
    for (i = 0; i < NUM_AULAS; ++i) {
        clave_aula[i] = CHECK(ftok(FILEKEY, BASEKEY_SHM + i));
        shm_aula[i] = CHECK(shmget(clave_aula[i], tamanio_aula[i]*sizeof(asiento_t), IPC_CREAT | IPC_EXCL));
        mem_aula[i] = check_mem(shmat(shm_aula[i], NULL, 0));
        memset(mem_aula[i], 0, tamanio_aula[i]*sizeof(asiento_t)); // Los asientos empiezan vacios

        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            clave_profesor[i][j] = CHECK(ftok(FILEKEY, BASEKEY_MSQ));
            msq_profesor[i][j] = CHECK(msgget(clave_profesor[i][j], IPC_CREAT | MSQ_PERMS));

            pid = CHECK(fork());
            if (pid == 0) {
                exit(profesor(i, clave_aula[i], clave_profesor[i][j]));
            }
        }
    }
 
    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].esperando = 1;
        estado_alumno[i].aula = i % NUM_AULAS;

        CHECK(pthread_create(&estado_alumno[i].hilo, NULL, alumno, &estado_alumno[i]));
    }

    // Cleanup

    for (i = 0; i < numero_alumnos; ++i) {
        CHECK(pthread_join(estado_alumno[i].hilo, NULL));
    }

    for (i = 0; i < NUM_AULAS*NUM_PROFS_POR_AULA; ++i) {
        CHECK(wait(NULL));
    }

    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(shmdt(mem_aula[i]));
        CHECK(shmctl(shm_aula[i], IPC_RMID, NULL));

        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(msgctl(clave_profesor[i][j], IPC_RMID, NULL));
        }
    }

    return 0;
}

void* alumno(void *arg) {
    alumno_state_t *estado = (alumno_state_t*)arg;

    printf("Probando aula %d\n", estado->aula);

    return 0;
}

int profesor(int aula, key_t clave_aula, key_t clave_msq) {
    return 0;
}

int check_err(const char *fname, int val) {
    if (val == -1) {
        char err_desc[256];
        if (strerror_r(errno, err_desc, sizeof(err_desc))) {
            fprintf(stderr, "PID:%d :: Fallo de %s: Error obteniendo descripcion de error\n", getpid(), fname);
            exit(1);
        }
        fprintf(stderr, "PID:%d :: Fallo de %s: errno = %s\n", getpid(), fname, err_desc);
        exit(1);
    }

    return val;
}

void* check_mem(void* mem) {
    if (mem == NULL) {
        fprintf(stderr, "Fallo en malloc\n");
        exit(1);
    }

    return mem;
}
