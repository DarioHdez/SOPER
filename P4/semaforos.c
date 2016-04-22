/**
 * @file semaforos.c 
 * @brief Implementa los semaforos

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/03/27
*/

#include "semaforos.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

int Operar_Semaforo(int semid, int size, int undo, int* active, int sum);

int Crear_Semaforo(key_t key, int size, int *semid){
  if (!semid)
    return ERROR;
  
  /*No se diferenciar cuando ya estaba creado de cuando no, edit: Puede que este solucionado*/
  /*Me lio con el puntero *semid, porque semget devuelve un int, ¿es * o &?*/
  *semid = semget(key, size, IPC_CREAT | SHM_R | SHM_W);  
  if ((*semid == -1) && errno == EEXIST) {
		*semid = semget(key, size, SHM_R|SHM_W);
  }

  if (*semid == -1) {
    return ERROR;
  }

  return OK;  
}

int Borrar_Semaforo(int semid){
  if (semid == -1) 
    return ERROR;

  /*¿Aqui no hace falta el 4o argumento?*/
  return semctl(semid, 0/*Ignorado*/, IPC_RMID, NULL);
}

int Inicializar_Semaforo(int semid, unsigned short* array){
  return semctl(semid, 0/*Ignorado*/, SETALL, array) == -1 ? ERROR : OK;
}

int Down_Semaforo(int semid, int num_sem, int undo){
  return DownMultiple_Semaforo(semid, 1, undo, &num_sem);
}

int DownMultiple_Semaforo(int semid, int size, int undo, int* active) {
  return Operar_Semaforo(semid, size, undo, active, -1);
}

int Up_Semaforo(int semid, int num_sem, int undo){
  return UpMultiple_Semaforo(semid, 1, undo, &num_sem);
}

int UpMultiple_Semaforo(int semid, int size, int undo, int* active){
  return Operar_Semaforo(semid, size, undo, active, +1);
}

int Operar_Semaforo(int semid, int size, int undo, int* active, int sum) {
  if (semid < 0 || size < 0 || !active)
    return ERROR;

  struct sembuf *sem_oper = (struct sembuf *)malloc(size*sizeof(struct sembuf));
  if (sem_oper == NULL) {
    return ERROR;
  }

  int op;
  for (op = 0; op < size; ++op) {
    sem_oper[op].sem_num = active[op];
    sem_oper[op].sem_op = sum;
    sem_oper[op].sem_flg = undo ? SEM_UNDO : 0;
  }

  if (semop(semid, sem_oper, size) == -1)
    return ERROR;

  free(sem_oper);

  return OK;  
}

