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

#define BUF_SIZE 1024

#define CHECK(f) (check_err(#f, __FILE__, __LINE__, (f), 1))
#define CHECK_GOTO(val, g) { if (check_err(#val, __FILE__, __LINE__, (val), 0) == -1) { goto g; } }

#define FILEKEY "/bin/cat"
#define KEY_SEM 24667
#define BASEKEY_SHM 34667
#define BASEKEY_MSQ 44667
#define MSQ_PERMS 0660

#define SIZEOF_AULA(tamanio_aula) (sizeof(aula_t) + (tamanio_aula)*sizeof(asiento_t))
#define NUMERO_SEMAFOROS (numero_alumnos*2)

typedef struct {
    int ocupado;
    pthread_t th;
} asiento_t;

typedef struct {
    int num_sem_mutex;
    int num_sem_sync;
    pthread_t th;
    int esperando;
    int aula;
} alumno_state_t;

typedef struct {
    long type;
    pthread_t th;
    int num_sem_sync;
} coloca_msg_t;

typedef struct {
    int capacidad;
    asiento_t asientos[1];
} aula_t;

int check_err(const char *fname, char *file, int line, int val, int exit_on_fail);
void* check_mem(void* mem);

int alumno_lock(alumno_state_t *state);
int alumno_unlock(alumno_state_t *state);
int alumno_esperar(alumno_state_t *state);
int alumno_despertar(alumno_state_t *state);

void* alumno(void *arg);
int profesor(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq);

alumno_state_t *estado_alumno;

int semid;

int main() {
    unsigned int tamanio_aula[NUM_AULAS];

    key_t clave_aula[NUM_AULAS];
    int shm_aula[NUM_AULAS];
    aula_t* mem_aula[NUM_AULAS];

    key_t clave_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];
    int msq_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];

    key_t clave_sem = CHECK(ftok(FILEKEY, KEY_SEM));

    unsigned int numero_alumnos;
    unsigned int i, j, k;

    // Inicializacion

    for (i = 0; i < NUM_AULAS; ++i) {
        printf("Introduzca el numero de asientos en aula %d: ", i + 1);
        scanf("%u", &tamanio_aula[i]);
    }

    printf("Introduzca el numero de alumnos: ");
    scanf("%u", &numero_alumnos);

    estado_alumno = (alumno_state_t*)check_mem(malloc(numero_alumnos*sizeof(alumno_state_t)));

    CHECK_GOTO(Crear_Semaforo(clave_sem, NUMERO_SEMAFOROS, &semid), fail_sem);
    unsigned short* inicializacion = (unsigned short*)check_mem(malloc(NUMERO_SEMAFOROS*sizeof(unsigned short)));
    
    for (i = 0; i < NUMERO_SEMAFOROS; ++i) {
        inicializacion[i] = i % 2; // Sync - Mutex
    }

    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].num_sem_sync = (2*i);
        estado_alumno[i].num_sem_mutex = (2*i) + 1;
        estado_alumno[i].esperando = 1;
    }

    CHECK(Inicializar_Semaforo(semid, inicializacion));

    free(inicializacion);

    int pid;
    for (i = 0; i < NUM_AULAS; ++i) {
        clave_aula[i] = CHECK(ftok(FILEKEY, BASEKEY_SHM + i));
        shm_aula[i] = shmget(clave_aula[i], SIZEOF_AULA(tamanio_aula[i]), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        CHECK_GOTO(shm_aula[i], fail_shm);
        mem_aula[i] = (aula_t*)shmat(shm_aula[i], NULL, 0);
        if (mem_aula[i] == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); exit(1); }
        mem_aula[i]->capacidad = tamanio_aula[i];
        memset(mem_aula[i]->asientos, 0, SIZEOF_AULA(tamanio_aula[i])); // Los asientos empiezan vacios

        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            clave_profesor[i][j] = CHECK(ftok(FILEKEY, BASEKEY_MSQ + i*NUM_PROFS_POR_AULA + j));
            msq_profesor[i][j] = msgget(clave_profesor[i][j], IPC_CREAT | MSQ_PERMS);
            CHECK_GOTO(msq_profesor[i][j], fail_msq);

            pid = CHECK(fork());
            if (pid == 0) {
                exit(profesor(i, clave_sem, clave_aula[i], clave_profesor[i][j]));
            }
        }
    }
 
    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].esperando = 1;
        estado_alumno[i].aula = i % NUM_AULAS;
        // estado_alumno[i].aula = 0;

        CHECK(pthread_create(&estado_alumno[i].th, NULL, alumno, &estado_alumno[i]));
    }

    // Principal
    
    {
        coloca_msg_t cm;
        for (i = 0; i < numero_alumnos; ++i) {
            int aula, snd = 0;

            cm.type = 1;

            CHECK(alumno_lock(&estado_alumno[i]));
                if(estado_alumno[i].esperando) {
                    cm.th = estado_alumno[i].th;
                    cm.num_sem_sync = estado_alumno[i].num_sem_sync;

                    estado_alumno[i].esperando = 0;

                    aula = estado_alumno[i].aula;
                    snd = 1;
                }
            CHECK(alumno_unlock(&estado_alumno[i]));

            if (snd) {
                CHECK(msgsnd(msq_profesor[aula][0], &cm, sizeof(coloca_msg_t) - sizeof(long), 0));
            }
        }
    }

    // Cleanup
    
    free(estado_alumno);

    for (i = 0; i < numero_alumnos; ++i) {
        CHECK(pthread_join(estado_alumno[i].th, NULL));
    }

    CHECK(Borrar_Semaforo(semid));

    for (i = 0; i < NUM_AULAS*NUM_PROFS_POR_AULA; ++i) {
        CHECK(wait(NULL));
    }

    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(shmdt(mem_aula[i]));
        CHECK(shmctl(shm_aula[i], IPC_RMID, NULL));

        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(msgctl(msq_profesor[i][j], IPC_RMID, NULL));
        }
    }

    return 0;

fail_msq:
    for (k = 0; k <= j; ++k) {
        CHECK(msgctl(clave_profesor[i][k], IPC_RMID, NULL));
    }
fail_shm:
    for (j = 0; j <= i; ++j) {
        CHECK(shmdt(mem_aula[j]));
        CHECK(shmctl(shm_aula[j], IPC_RMID, NULL));

        for (k = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(msgctl(clave_profesor[j][k], IPC_RMID, NULL));
        }
    }

    CHECK(Borrar_Semaforo(semid));
fail_sem:
    free(estado_alumno);

    return 1;
}

int alumno_lock(alumno_state_t *state) {
    return Down_Semaforo(semid, state->num_sem_mutex, 1);
}

int alumno_unlock(alumno_state_t *state) {
    return Up_Semaforo(semid, state->num_sem_mutex, 1);
}

int alumno_esperar(alumno_state_t *state) {
    return Down_Semaforo(semid, state->num_sem_sync, 1);
}

int alumno_despertar(alumno_state_t *state) {
    return Up_Semaforo(semid, state->num_sem_sync, 1);
}

void* alumno(void *arg) {
    alumno_state_t *estado = (alumno_state_t*)arg;
    
    int id = estado->num_sem_sync/2;

    CHECK(alumno_lock(estado));
        printf("alumno %d: Probando aula %d\n", id, estado->aula);
    CHECK(alumno_unlock(estado));

    CHECK(alumno_esperar(estado));

    CHECK(alumno_lock(estado));
        estado->esperando = 0;
    CHECK(alumno_unlock(estado));

    printf("alumno %d: Espera terminada\n", id);

    return 0;
}

int profesor(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq) {
    int shm_aula;
    aula_t *mem_aula;
    int msq;
    char buf[BUF_SIZE];

    CHECK(Crear_Semaforo(clave_sem, 0, &semid));

    shm_aula = CHECK(shmget(clave_aula, 0, SHM_R | SHM_W));
    mem_aula = (aula_t*)shmat(shm_aula, NULL, 0);
    if (mem_aula == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); return 1; }
    msq = CHECK(msgget(clave_msq, 0));

    for(;;) {
        int bytes_read = CHECK(msgrcv(msq, &buf, BUF_SIZE - sizeof(long), 0, 0));

        if (bytes_read + sizeof(long) < sizeof(coloca_msg_t)) {
            fprintf(stderr, "Mensaje demasiado pequeño\n");
            break;
        }

        coloca_msg_t *cm = (coloca_msg_t*)buf;

        printf("Profe: Colocando alumno %d\n", cm->num_sem_sync/2);
        
        int i;
        for (i = 0; i < mem_aula->capacidad; ++i) {
            if (mem_aula->asientos[i].ocupado == 0) {
                mem_aula->asientos[i].ocupado = 1;
                mem_aula->asientos[i].th = cm->th;
                printf("Profe: Alumno %d colocado en asiento %d\n", cm->num_sem_sync, i);
                CHECK(Up_Semaforo(semid, cm->num_sem_sync, 1));
                break;
            }
        }
    }

    CHECK(shmdt(mem_aula));

    return 0;
}

int check_err(const char *fname, char *file, int line, int val, int exit_on_fail) {
    if (val == -1) {
        char err_desc[256];
        if (strerror_r(errno, err_desc, sizeof(err_desc))) {
            fprintf(stderr, "%s@%d PID:%d :: Fallo de %s: Error obteniendo descripcion de error\n", file, line, getpid(), fname);
            if (exit_on_fail) {
                exit(1);
            } else {
                return val;
            }
        }
        fprintf(stderr, "%s@%d PID:%d :: Fallo de %s: errno = %s\n", file, line, getpid(), fname, err_desc);
        if (exit_on_fail) {
            exit(1);
        } else {
            return val;
        }
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
