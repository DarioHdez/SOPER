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
#define NUM_SEMAFOROS (NUM_AULAS + (numero_alumnos*2))

typedef struct {
    int ocupado;
    pthread_t th;
} asiento_t;

typedef struct {
    int num_sem_mutex;
    int num_sem_sync;
    pthread_t th;
    int muevete;
    int esperando;
    int aula;
} alumno_state_t;

typedef struct {
    long type;
    pthread_t th;
    int num_sem_sync;
} coloca_msg_t;

typedef struct {
    int num_sem_aula_mutex;
    int capacidad;
    int ocupacion;
    asiento_t asientos[1];
} aula_t;

int check_err(const char *fname, char *file, int line, int val, int exit_on_fail);
void* check_mem(void* mem);

int alumno_lock(alumno_state_t *state);
int alumno_unlock(alumno_state_t *state);
int alumno_esperar(alumno_state_t *state);
int alumno_despertar(alumno_state_t *state);
void alumno_debug(alumno_state_t *state);

int aula_lock(aula_t *aula);
int aula_unlock(aula_t *aula);

void* alumno(void *arg);
int profesor(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq);

void handler_sigterm();

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
    pid_t profesor_pid[NUM_AULAS][NUM_PROFS_POR_AULA];

    // Inicializacion

    for (i = 0; i < NUM_AULAS; ++i) {
        printf("Introduzca el numero de asientos en aula %d: ", i + 1);
        scanf("%u", &tamanio_aula[i]);
    }

    printf("Introduzca el numero de alumnos: ");
    scanf("%u", &numero_alumnos);

    estado_alumno = (alumno_state_t*)check_mem(malloc(numero_alumnos*sizeof(alumno_state_t)));

    CHECK_GOTO(Crear_Semaforo(clave_sem, NUM_SEMAFOROS, &semid), fail_sem);
    unsigned short* inicializacion = (unsigned short*)check_mem(malloc(NUM_SEMAFOROS*sizeof(unsigned short)));
    
    for (i = 0; i < NUM_AULAS; ++i) {
        inicializacion[i] = 1;
    }

    for (i = NUM_AULAS; i < NUM_SEMAFOROS; ++i) {
        inicializacion[i] = i % 2; // Sync - Mutex
    }

    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].num_sem_sync = NUM_AULAS + (2*i);
        estado_alumno[i].num_sem_mutex = NUM_AULAS + (2*i) + 1;
    }

    CHECK(Inicializar_Semaforo(semid, inicializacion));

    free(inicializacion);

    for (i = 0; i < NUM_AULAS; ++i) {
        clave_aula[i] = CHECK(ftok(FILEKEY, BASEKEY_SHM + i));
        shm_aula[i] = shmget(clave_aula[i], SIZEOF_AULA(tamanio_aula[i]), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
        CHECK_GOTO(shm_aula[i], fail_shm);
        mem_aula[i] = (aula_t*)shmat(shm_aula[i], NULL, 0);
        if (mem_aula[i] == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); exit(1); }

        mem_aula[i]->num_sem_aula_mutex = i;
        mem_aula[i]->capacidad = tamanio_aula[i];
        mem_aula[i]->ocupacion = 0;
        memset(mem_aula[i]->asientos, 0, SIZEOF_AULA(tamanio_aula[i])); // Los asientos empiezan vacios

        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            clave_profesor[i][j] = CHECK(ftok(FILEKEY, BASEKEY_MSQ + i*NUM_PROFS_POR_AULA + j));
            msq_profesor[i][j] = msgget(clave_profesor[i][j], IPC_CREAT | MSQ_PERMS);
            CHECK_GOTO(msq_profesor[i][j], fail_msq);

            profesor_pid[i][j] = CHECK(fork());
            if (profesor_pid[i][j] == 0) {
                exit(profesor(i, clave_sem, clave_aula[i], clave_profesor[i][j]));
            }
        }
    }
 
    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].muevete = 0;
        estado_alumno[i].esperando = 1;
        estado_alumno[i].aula = i % NUM_AULAS;

        CHECK(pthread_create(&estado_alumno[i].th, NULL, alumno, &estado_alumno[i]));
    }

    // Principal
    
    for (;;) {
        printf("BUCLE!\n");

        for (i = 0; i < numero_alumnos; ++i) {
            int aula;

            CHECK(alumno_lock(&estado_alumno[i]));
                if(estado_alumno[i].esperando) {
                    coloca_msg_t cm;
                    memset(&cm, 0, sizeof(cm)); //Calm valgrind
                    cm.type = 1;
                    cm.th = estado_alumno[i].th;
                    cm.num_sem_sync = estado_alumno[i].num_sem_sync;

                    aula = estado_alumno[i].aula;
                    CHECK(msgsnd(msq_profesor[aula][0], &cm, sizeof(coloca_msg_t) - sizeof(long), 0));
                }
            CHECK(alumno_unlock(&estado_alumno[i]));

            for (j = 0; j < NUM_AULAS; ++j) {
                CHECK(aula_lock(mem_aula[j]));
                    int necesito_mover = mem_aula[j]->ocupacion > (float)mem_aula[j]->capacidad*0.85;
                    if (necesito_mover) {
                        for (k = 0; k < numero_alumnos; ++k) {
                            CHECK(alumno_lock(&estado_alumno[k]));
                                if (estado_alumno[k].aula == j && estado_alumno[k].esperando) {
                                    estado_alumno[k].muevete = 1;
                                    printf("Gestor: Mandando mover a alumno %d\n", estado_alumno[k].num_sem_sync / 2 - 1);
                                    CHECK(alumno_despertar(&estado_alumno[k]));
                                    necesito_mover = 0;
                                }
                            CHECK(alumno_unlock(&estado_alumno[k]));
                        }
                    }
                CHECK(aula_unlock(mem_aula[j]));
            }
        }


        int esperando = 0;

        for (k = 0; k < numero_alumnos; ++k) {
            CHECK(alumno_lock(&estado_alumno[k]));
                if (estado_alumno[k].esperando) { 
                    esperando++;
                }
            CHECK(alumno_unlock(&estado_alumno[k]));
        }

        printf("esperando = %d\n", esperando);

        if (esperando == 0) {
            break;
        }
    }

    // Cleanup

    for (i = 0; i < numero_alumnos; ++i) {
        CHECK(pthread_join(estado_alumno[i].th, NULL));
    }

    free(estado_alumno);

    CHECK(Borrar_Semaforo(semid));

    for (i = 0; i < NUM_AULAS; ++i) {
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            kill(profesor_pid[i][j], SIGTERM);
        }
    }

    for (i = 0; i < NUM_AULAS; ++i) {
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(wait(NULL));
        }
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
    int ret = Up_Semaforo(semid, state->num_sem_sync, 1);
    return ret;
}

void alumno_debug(alumno_state_t *state) {
    printf("num_sem_mutex = %d\nnum_sem_sync = %d\n", state->num_sem_mutex, state->num_sem_sync);
}

void* alumno(void *arg) {
    alumno_state_t *estado = (alumno_state_t*)arg;
    
    int id = estado->num_sem_sync/2 - 1;

    CHECK(alumno_lock(estado));
        printf("alumno %d: Probando aula %d\n", id, estado->aula + 1);
    CHECK(alumno_unlock(estado));

    int bucle = 1;
    while (bucle) {
        CHECK(alumno_esperar(estado));

        CHECK(alumno_lock(estado));
            if (estado->muevete) {
                estado->aula = (estado->aula + 1) % NUM_AULAS;
                printf("alumno %d: Me muevo a aula %d\n", id, estado->aula + 1);
                estado->muevete = 0;
            } else {
                estado->esperando = 0;
                printf("alumno %d: Espera terminada\n", id);
                bucle = 0;
            }
        CHECK(alumno_unlock(estado));
    }

    return 0;
}

int aula_lock(aula_t *aula) {
    return Down_Semaforo(semid, aula->num_sem_aula_mutex, 1);
}

int aula_unlock(aula_t *aula) {
    return Up_Semaforo(semid, aula->num_sem_aula_mutex, 1);
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

    signal(SIGTERM, handler_sigterm);


    for (;;) {
        int bytes_read = msgrcv(msq, &buf, BUF_SIZE - sizeof(long), 0, 0);
        CHECK_GOTO(bytes_read, msgrcv_err)

        if (bytes_read + sizeof(long) < sizeof(coloca_msg_t)) {
            fprintf(stderr, "Mensaje demasiado pequeño\n");
            break;
        }

        coloca_msg_t *cm = (coloca_msg_t*)buf;

        int id_alum = cm->num_sem_sync/2 - 1;

        CHECK(aula_lock(mem_aula));

            printf("Profe %d: Colocando alumno %d\n", getpid(), id_alum);
            
            int i;
            for (i = 0; i < mem_aula->capacidad; ++i) {
                if (mem_aula->asientos[i].ocupado == 0) {
                    mem_aula->asientos[i].ocupado = 1;
                    mem_aula->asientos[i].th = cm->th;

                    mem_aula->ocupacion++;
                    printf("Profe %d: ocupacion aula %d = %d\n", getpid(), aula + 1, mem_aula->ocupacion);

                    printf("Profe %d: Alumno %d colocado en asiento %d(aula %d)\n", getpid(), id_alum, i, aula + 1);
                    CHECK(Up_Semaforo(semid, cm->num_sem_sync, 1));
                    break;
                }
            }

            if (i == mem_aula->capacidad) {
                printf("Profe %d: No hay asiento para alumno %d en aula %d\n", getpid(), id_alum, aula + 1);
            }

        CHECK(aula_unlock(mem_aula));
    }

    CHECK(shmdt(mem_aula));

    return 0;

msgrcv_err:
    if (errno == EINTR) {
        CHECK(shmdt(mem_aula));
        return 0;
    }

    return 1;
}


void handler_sigterm(){
    printf("Profe %d: acaba su turno\n", getpid());

    free(estado_alumno);

    exit(0);
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
