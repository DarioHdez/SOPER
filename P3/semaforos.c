/**
 * @file ejercicio4.c 
 * @brief Implementa los semaforos

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/03/26
*/

#include "semaforos.h"


int Inicializar_Semaforo(int semid, unsigned short* array){
  if (semid == -1)
		return ERROR;
	
  int i = 0;
  
  /*Mientras el array tenga numeros*/
  /*En teoria array tiene que ser una estructura, asi que no 
    estoy seguro de que esto funcione*/
  while (array[i] != NULL) {  
    if (semctl(semid,i, SETVAL, array[i]) == -1)
      return ERROR; 	
	}
  
  return OK;
}

int Borrar_Semaforo(int semid){
  if (semid == -1) 
    return ERROR;

  /*¿Aqui no hace falta el 4o argumento?*/
  return semctl(semid,0,IPC_RMID);
}

int Crear_Semaforo(key_t key, int size, int *semid){
  if (key < 0 || size < 1)
    return ERROR;
  
  /*No se diferenciar cuando ya estaba creado de cuando no*/
  &semid = semget(key,size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);  
  if (&semid == -1)
    return ERROR;  

  return OK;  
}

int Down_Semaforo(int semid, int num_sem, int undo){
  if (semid < 0 || num_sem < 0)
    return ERROR;

  struct sembuf sem_oper;

  sem_oper.sem_num = num_sem;
  sem_oper.sem_op = -1;
  sem_oper.sem_flg = undo;
  
  if (semop(semid, &sem_oper, 1) == -1)
    return ERROR;
  
  return OK;
}

int DownMultiple_semaforo(int semid, int size, int undo, int* active){
  if (semid < 0 || size < 0 || !active)
    return ERROR;

  struct sembuf sem_oper;

  sem_oper.sem_num = size;
  sem_oper.sem_op = -1;
  sem_oper.sem_flg = undo;

  if (semop(semid, &sem_oper, &active) == -1)
    return ERROR;

  return OK;  
}

int Up_Semaforo(int semid, int num_sem, int undo){
  if (semid < 0 || num_sem < 0)
    return ERROR;

  struct sembuf sem_oper;

  sem_oper.sem_num = num_sem;
  sem_oper.sem_op = +1;
  sem_oper.sem_flg = undo;

  if (semop(semid, &sem_oper, 1) == -1)
    return ERROR;

  return OK;
}

int UpMultiple_Semaforo(int semid, int size, int undo, int* active){
  if (semid < 0 || size < 0 || !active)
    return ERROR;

  struct sembuf sem_oper;

  sem_oper.sem_num = size;
  sem_oper.sem_op = +1;
  sem_oper.sem_flg = undo;

  if (semop(semid, &sem_oper, &active) == -1)
    return ERROR;

  return OK;
}
