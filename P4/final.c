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
#include <signal.h>

#include <pthread.h>

#include "semaforos.h"

/*Macros*/

#define NUM_AULAS 2
#define NUM_PROFS_POR_AULA 2

#define BUF_SIZE 1024

#define CHECK(f) (check_err(# f, __FILE__, __LINE__, (f), 1))

#define FILEKEY "/bin/cat"
#define KEY_SEM 24667
#define BASEKEY_SHM 34667
#define BASEKEY_MSQ 44667
#define MSQ_PERMS 0660

#define SIZEOF_AULA(tamanio_aula) (sizeof(aula_t) + (tamanio_aula)*sizeof(asiento_t))
#define NUM_SEMAFOROS (NUM_AULAS + (numero_alumnos*2))

#define ALUMNO_ESPERA_MAXIMA 10
#define ALUMNO_EXAMEN_MAXIMO 60
#define ESPERA_CAMPANA (1*60)

/*TAD asiento*/
typedef struct {
    int ocupado;
    pthread_t th;
} asiento_t;

typedef enum {
    ALUMNO_EN_CALLE,
    ALUMNO_EN_PASILLO,
    ALUMNO_MOVIENDOSE,
    ALUMNO_SENTANDOSE,
    ALUMNO_SENTADO,
    ALUMNO_LEVANTANDOSE,
    ALUMNO_EXAMINADO
} alumno_status_t;

/*TAD alumno*/
typedef struct {
    int num_sem_mutex;
    int num_sem_sync;
    pthread_t th;
    alumno_status_t status;
    int aula;
} alumno_state_t;

/*Cola de mensajes, del gestor a los profesores para sentar alumnos*/
typedef struct {
    long type;              /*Tipo de mensaje*/
    pthread_t th;           /*Alumno a sentar*/
    int num_sem_sync;       /*Numero de semaforo del alumno*/
} coloca_msg_t;

/*TAD aula*/
typedef struct {
    int num_sem_aula_mutex;  /*Numero de semaforo del aula*/
    int capacidad;           /*Capacidad total*/
    int ocupacion;           /*Ocupacion durante la ejecucion*/
    asiento_t asientos[1];   /*Array de asientos en el aula*/
} aula_t;

/*Funciones de comprobacion de errores*/
int check_err(const char *fname, char *file, int line, int val, int exit_on_fail);
void* check_mem(void* mem);

/*Funciones para operar los semaforos de los alumnos*/
int alumno_lock(alumno_state_t *state);
int alumno_unlock(alumno_state_t *state);
int alumno_esperar(alumno_state_t *state);
int alumno_despertar(alumno_state_t *state);

/*Funcion para debugear el alumno y ver/corregir fallos*/
void alumno_debug(alumno_state_t *state);

/*Funciones para operar con los semaforos de las aulas*/
int aula_lock(aula_t *aula);
int aula_unlock(aula_t *aula);

/*Funcion que ejecuta el hilo alumno*/
void* alumno(void *arg);

/*Funcion que ejecutan los procesos profesor*/
int profesor(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq);

int examinador(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq);

/*Funcion que ejecuta el proceso que muestra el ps*/
int vigilante();

/*Handler de sigterm para los profesores*/
void handler_sigterm();

/*Handler de SIG_ALRM para imprimir ps*/
void handler_sigalrm();

/*Campanazo*/
void campana();

void aula_print(aula_t *aula);

/*Variables globales*/
alumno_state_t *estado_alumno;

int semid;

unsigned int numero_alumnos;
int msq_examinador[NUM_AULAS];
aula_t* mem_aula[NUM_AULAS];

/*Programa principal*/
int main() {
    unsigned int tamanio_aula[NUM_AULAS];

    /*Variables para reservar la memoria compartida */
    key_t clave_aula[NUM_AULAS];
    int shm_aula[NUM_AULAS];

    /*Variables para la cola de mensajes de los profesores*/
    key_t clave_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];
    int msq_profesor[NUM_AULAS][NUM_PROFS_POR_AULA];

    key_t clave_examinador[NUM_AULAS];

    /*Variables para reservar los semaforos*/
    key_t clave_sem = CHECK(ftok(FILEKEY, KEY_SEM));

    /*Variables para los alumnos, bucles y profesores*/
    unsigned int i, j, k;
    pid_t profesor_pid[NUM_AULAS][NUM_PROFS_POR_AULA];
    pid_t vigilante_pid;
    pid_t examinador_pid[NUM_AULAS];

    /*INICIO DEL PROGRAMA*/

    /*Pedimos al usuario que introduzca el numero de asientos por aula*/
    for (i = 0; i < NUM_AULAS; ++i) {
        printf("Introduzca el numero de asientos en aula %d: ", i + 1);
        scanf("%u", &tamanio_aula[i]);
    }

    /*Numero de alumnos que entraran en la simulacion*/
    printf("Introduzca el numero de alumnos: ");
    scanf("%u", &numero_alumnos);

    vigilante_pid = CHECK(fork());
    if (vigilante_pid == 0) {
        exit(vigilante());
    }

    /*Reservamos memoria para los alumnos*/
    estado_alumno = (alumno_state_t*)check_mem(malloc(numero_alumnos*sizeof(alumno_state_t)));

    /*Creamos los semaforos*/
    CHECK(Crear_Semaforo(clave_sem, NUM_SEMAFOROS, &semid));
    unsigned short* inicializacion = (unsigned short*)check_mem(malloc(NUM_SEMAFOROS*sizeof(unsigned short)));

    /*Bucles de inicializacion de semaforos*/
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

    /*Inicializamos los semaforos a traves de nuestra libreria*/
    CHECK(Inicializar_Semaforo(semid, inicializacion));

    /*Dado que los semaforos ya están inicializados, nos sobra esta memoria reservada*/
    free(inicializacion);

    /*CREACION/INICIALIZACION DE LAS AULAS*/
    for (i = 0; i < NUM_AULAS; ++i) {
        /*Memoria compartida*/
        clave_aula[i] = CHECK(ftok(FILEKEY, BASEKEY_SHM + i));
        shm_aula[i] = CHECK(shmget(clave_aula[i], SIZEOF_AULA(tamanio_aula[i]), IPC_CREAT | IPC_EXCL | SHM_R | SHM_W));
        mem_aula[i] = (aula_t*)shmat(shm_aula[i], NULL, 0);
        if (mem_aula[i] == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); exit(1); }

        mem_aula[i]->num_sem_aula_mutex = i;
        mem_aula[i]->capacidad = tamanio_aula[i];
        mem_aula[i]->ocupacion = 0;
        memset(mem_aula[i]->asientos, 0, SIZEOF_AULA(tamanio_aula[i])); /* Los asientos empiezan vacios */

        clave_examinador[i] = CHECK(ftok(FILEKEY, BASEKEY_MSQ + i));
        msq_examinador[i] = CHECK(msgget(clave_examinador[i], IPC_CREAT | MSQ_PERMS));

        examinador_pid[i] = CHECK(fork());
        if (examinador_pid[i] == 0) {
            exit(examinador(i, clave_sem, clave_aula[i], clave_examinador[i]));
        }

        /*Colas de mensajes de los profesores*/
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            clave_profesor[i][j] = CHECK(ftok(FILEKEY, BASEKEY_MSQ + NUM_AULAS + i*NUM_PROFS_POR_AULA + j));
            msq_profesor[i][j] = CHECK(msgget(clave_profesor[i][j], IPC_CREAT | MSQ_PERMS));

            /*Creamos los procesos profesor*/
            profesor_pid[i][j] = CHECK(fork());
            if (profesor_pid[i][j] == 0) {
                exit(profesor(i, clave_sem, clave_aula[i], clave_profesor[i][j]));
            }
        }
    }

    signal(SIGALRM, campana);
    alarm(ESPERA_CAMPANA);

    /*Creamos los hilos alumno*/
    for (i = 0; i < numero_alumnos; ++i) {
        estado_alumno[i].status = ALUMNO_EN_CALLE;
        estado_alumno[i].aula = i % NUM_AULAS;

        CHECK(pthread_create(&estado_alumno[i].th, NULL, alumno, &estado_alumno[i]));
    }

    /*FUNCION PRINCIPAL DEL PROCESO GESTOR*/

    for (;; ) {
        /*El gestor va mandando alumnos a un aula a hacer el examen*/
        for (i = 0; i < numero_alumnos; ++i) {
            int aula;

            CHECK(alumno_lock(&estado_alumno[i]));
            if(estado_alumno[i].status == ALUMNO_EN_PASILLO) {
                coloca_msg_t cm;
                memset(&cm, 0, sizeof(cm));     //Calm valgrind
                cm.type = 1;
                cm.th = estado_alumno[i].th;
                cm.num_sem_sync = estado_alumno[i].num_sem_sync;

                aula = estado_alumno[i].aula;

                aula_lock(mem_aula[aula]);
                if (mem_aula[aula]->ocupacion < mem_aula[aula]->capacidad) {
                    estado_alumno[i].status = ALUMNO_SENTANDOSE;

                    mem_aula[aula]->ocupacion++;

                    /*Manda un mensaje al profesor de que meta a un alumno a hacer el examen*/
                    CHECK(msgsnd(msq_profesor[aula][rand() % NUM_PROFS_POR_AULA], &cm, sizeof(coloca_msg_t) - sizeof(long), 0));
                }
                aula_unlock(mem_aula[aula]);
            }
            CHECK(alumno_unlock(&estado_alumno[i]));
        }

        sleep(1);

        /*En este bucle el gestor mira si hay algun aula a más del 85% y movemos alumnos en funcion de eso*/
        for (j = 0; j < NUM_AULAS; ++j) {
            CHECK(aula_lock(mem_aula[j]));
            int necesito_mover = mem_aula[j]->ocupacion >= (float)mem_aula[j]->capacidad*0.85;
            //int necesito_mover = mem_aula[j]->ocupacion >= mem_aula[j]->capacidad;

            if (necesito_mover) {
                for (k = 0; k < numero_alumnos; ++k) {
                    CHECK(alumno_lock(&estado_alumno[k]));
                    if (estado_alumno[k].aula == j && estado_alumno[k].status == ALUMNO_EN_PASILLO) {
                        estado_alumno[k].status = ALUMNO_MOVIENDOSE;
                        CHECK(alumno_despertar(&estado_alumno[k]));
                        necesito_mover = 0;
                    }
                    CHECK(alumno_unlock(&estado_alumno[k]));
                }
            }
            CHECK(aula_unlock(mem_aula[j]));
        }

        int sin_terminar = 0;

        /*En este bucle el gestor mira si quedan alumnos esperando para hacer el examen*/
        for (k = 0; k < numero_alumnos; ++k) {
            CHECK(alumno_lock(&estado_alumno[k]));
            if (estado_alumno[k].status != ALUMNO_EXAMINADO) {
                sin_terminar++;
            }
            CHECK(alumno_unlock(&estado_alumno[k]));
        }

        printf("Examinados: %d/%d\n", numero_alumnos - sin_terminar, numero_alumnos);

        for (k = 0; k < NUM_AULAS; ++k) {
            aula_print(mem_aula[k]);
        }

        /*Cuando no queden alumnos esperando para hacer el examen acabo mi funcion de gestor*/
        if (sin_terminar == 0) {
            break;
        }
    }

    /*LIBERAMOS TODOS LOS RECURSOS UTILIZADOS*/

    /*Recogemos los hilos alumno*/
    for (i = 0; i < numero_alumnos; ++i) {
        CHECK(pthread_join(estado_alumno[i].th, NULL));
    }
    /*Liberamos su memoria*/
    free(estado_alumno);

    /*Borramos los semaforos*/
    CHECK(Borrar_Semaforo(semid));

    CHECK(kill(vigilante_pid, SIGTERM));

    /*Mandamos la señal SIGTERM a los procesos profesor*/
    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(kill(examinador_pid[i], SIGTERM));
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(kill(profesor_pid[i][j], SIGTERM));
        }
    }

    /*Esperamos por los procesos profesor*/
    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(wait(NULL));
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(wait(NULL));
        }
    }

    /*Liberamos la memoria compartida*/
    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(shmdt(mem_aula[i]));
        CHECK(shmctl(shm_aula[i], IPC_RMID, NULL));

        CHECK(msgctl(msq_examinador[i], IPC_RMID, NULL));
        for (j = 0; j < NUM_PROFS_POR_AULA; ++j) {
            CHECK(msgctl(msq_profesor[i][j], IPC_RMID, NULL));
        }
    }

    return 0;   /*Acabamos el programa*/
}

void campana() {
    int i, j;

    printf("CAMPANA SONANDO\n");

    for (i = 0; i < NUM_AULAS; ++i) {
        CHECK(aula_lock(mem_aula[i]));
        for (j = 0; j < mem_aula[i]->capacidad; ++j) {
            if (mem_aula[i]->asientos[j].ocupado) {
                pthread_t alumno = mem_aula[i]->asientos[j].th;
                if (pthread_cancel(alumno) != 0) {
                    fprintf(stderr, "Error con pthread_cancel(alumno)\n");
                    exit(1);
                }

                mem_aula[i]->asientos[j].ocupado = 0;
                mem_aula[i]->ocupacion--;
            }
        }
        CHECK(aula_unlock(mem_aula[i]));
    }

    for (i = 0; i < numero_alumnos; ++i) {
        alumno_lock(&estado_alumno[i]);
        estado_alumno[i].status = ALUMNO_EXAMINADO;
        alumno_unlock(&estado_alumno[i]);
    }
}

void aula_print(aula_t *aula) {
    int id_aula = aula->num_sem_aula_mutex + 1;
    int i;

    CHECK(aula_lock(aula));
    printf("Aula %d: %d/%d\n\n", id_aula, aula->ocupacion, aula->capacidad);
    for (i = 0; i < aula->capacidad; ++i) {
        printf("%c%c", aula->asientos[i].ocupado ? 'O' : '_', (i + 1) % 11 ? ' ' : '\n');
    }
    printf("\n\n");
    CHECK(aula_unlock(aula));
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
    /*Permitimos cancelarnos en mitad del sleep*/
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    alumno_state_t *estado = (alumno_state_t*)arg;

    //int id = estado->num_sem_sync/2 - 1;

    sleep(rand() % ALUMNO_ESPERA_MAXIMA);

    CHECK(alumno_lock(estado));
    estado->status = ALUMNO_EN_PASILLO;
    CHECK(alumno_unlock(estado));

    int bucle = 1;
    while (bucle) {
        /*El alumno espera hasta que el profesor le de paso*/
        CHECK(alumno_esperar(estado));

        /*Si el alumno no se tiene que mover hace el examen y sale del bucle*/
        CHECK(alumno_lock(estado));
        if (estado->status == ALUMNO_MOVIENDOSE) {
            estado->aula = (estado->aula + 1) % NUM_AULAS;
            estado->status = ALUMNO_EN_PASILLO;
        } else if (estado->status == ALUMNO_SENTANDOSE) {
            estado->status = ALUMNO_SENTADO;
            bucle = 0;
        }
        CHECK(alumno_unlock(estado));
    }

    sleep(rand() % ALUMNO_EXAMEN_MAXIMO);

    estado->status = ALUMNO_LEVANTANDOSE;

    coloca_msg_t cm;
    memset(&cm, 0, sizeof(cm));
    cm.type = 2;
    cm.num_sem_sync = estado->num_sem_sync;
    cm.th = estado->th;

    CHECK(msgsnd(msq_examinador[estado->aula], &cm, sizeof(cm) - sizeof(long), 0));

    CHECK(alumno_esperar(estado));

    CHECK(alumno_lock(estado));
    estado->status = ALUMNO_EXAMINADO;
    CHECK(alumno_unlock(estado));

    return 0; /*Termina*/
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

    /*El profesor se une a los semaforos*/
    CHECK(Crear_Semaforo(clave_sem, 0, &semid));

    /*El profesor se une a la memoria compartida*/
    shm_aula = CHECK(shmget(clave_aula, 0, SHM_R | SHM_W));
    mem_aula = (aula_t*)shmat(shm_aula, NULL, 0);
    if (mem_aula == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); return 1; }
    msq = CHECK(msgget(clave_msq, 0));

    signal(SIGTERM, handler_sigterm);


    for (;; ) {
        /*Recibe/espera el mensaje del gestor de recoger un alumno*/
        int bytes_read = CHECK(msgrcv(msq, &buf, BUF_SIZE - sizeof(long), 0, 0));

        if (bytes_read + sizeof(long) < sizeof(coloca_msg_t)) {
            fprintf(stderr, "Mensaje demasiado pequeño\n");
            break;
        }

        coloca_msg_t *cm = (coloca_msg_t*)buf;

        int id_alum = cm->num_sem_sync/2 - 1;

        CHECK(aula_lock(mem_aula));
        /*Sienta al alumno y actualiza el aula*/

        int i;
        for (i = 0; i < mem_aula->capacidad; ++i) {
            if (mem_aula->asientos[i].ocupado == 0) {
                mem_aula->asientos[i].ocupado = 1;
                mem_aula->asientos[i].th = cm->th;

                //mem_aula->ocupacion++;

                CHECK(Up_Semaforo(semid, cm->num_sem_sync, 1));
                break;
            }
        }

        /*Comprueba si entran mas alumnos en el aula*/
        if (i == mem_aula->capacidad) {
            printf("Profe %d: No hay asiento para alumno %d en aula %d\n", getpid(), id_alum, aula + 1);
        }

        CHECK(aula_unlock(mem_aula));
    }

    CHECK(shmdt(mem_aula)); //Se quita de la memoria compartida

    return 0;
}

int examinador(int aula, key_t clave_sem, key_t clave_aula, key_t clave_msq) {
    int shm_aula;
    aula_t *mem_aula;
    int msq;
    char buf[BUF_SIZE];

    /*El profesor se une a los semaforos*/
    CHECK(Crear_Semaforo(clave_sem, 0, &semid));

    /*El profesor se une a la memoria compartida*/
    shm_aula = CHECK(shmget(clave_aula, 0, SHM_R | SHM_W));
    mem_aula = (aula_t*)shmat(shm_aula, NULL, 0);
    if (mem_aula == (void*)-1) { fprintf(stderr, "Fallo en shmat\n"); return 1; }
    msq = CHECK(msgget(clave_msq, 0));

    signal(SIGTERM, handler_sigterm);

    for (;; ) {
        /*Recibe/espera el mensaje del gestor de recoger un alumno*/
        int bytes_read = CHECK(msgrcv(msq, &buf, BUF_SIZE - sizeof(long), 0, 0));

        if (bytes_read + sizeof(long) < sizeof(coloca_msg_t)) {
            fprintf(stderr, "Mensaje demasiado pequeño\n");
            break;
        }

        coloca_msg_t *cm = (coloca_msg_t*)buf;

        int id_alum = cm->num_sem_sync/2 - 1;

        CHECK(aula_lock(mem_aula));
        /*Sienta al alumno y actualiza el aula*/

        int i;
        for (i = 0; i < mem_aula->capacidad; ++i) {
            if (mem_aula->asientos[i].ocupado == 1 && mem_aula->asientos[i].th == cm->th) {
                mem_aula->asientos[i].ocupado = 0;

                mem_aula->ocupacion--;

                CHECK(Up_Semaforo(semid, cm->num_sem_sync, 1));
                break;
            }
        }

        /*Comprueba si entran mas alumnos en el aula*/
        if (i == mem_aula->capacidad) {
            printf("examinador %d: Alumno %d no encontrado en aula %d\n", getpid(), id_alum, aula + 1);
        }

        CHECK(aula_unlock(mem_aula));
    }

    CHECK(shmdt(mem_aula)); //Se quita de la memoria compartida

    return 0;
}


void handler_sigterm() {
    /*Avisa de que ha acabado su turno, libera memoria y acaba*/
    printf("Profe %d: acaba su turno\n", getpid());

    free(estado_alumno);

    exit(0);
}

void handler_sigalrm() {
    pid_t gestor = getppid();
    int fd = CHECK(open("SIGHUP_PPID_lista_proc.txt",  O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR));

    int p[2];
    CHECK(pipe(p));
    if (CHECK(fork()) == 0) {
        char cmd[100];
        char* prog_args[] = { "sh", "-c", cmd, NULL };

        sprintf(cmd, "echo LISTADO DE PROCESOS && (ps -ef | grep %d | grep -v grep)", gestor);

        CHECK(dup2(fd, STDOUT_FILENO));

        CHECK(execvp("sh", prog_args));
        perror("execvp");
    }
}

void handler_sigterm_vigilante() {
    printf("Vigilante %d: acaba\n", getpid());

    exit(0);
}

int vigilante() {
    signal(SIGTERM, handler_sigterm_vigilante);
    signal(SIGALRM, handler_sigalrm);

    for (;; ) {
        alarm(3);
        pause();
    }

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
