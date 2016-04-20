/**
 * @file semaforos.h 
 * @brief Define un semáforo

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/03/26
*/

#ifndef SEMAFOROS_H
#define SEMAFOROS_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

#define OK 0
#define ERROR 1

union semun {
	int val;
	struct semid_ds *semstat;
	unsigned short *array;
}arg;

/**
  * @funcion Crear_Semaforo
  * @date 2016/03/26
  * @brief Crea un semaforo con la clave y el tamaño especificado. lo inicializa a 0.
  * @param key_t key: Clave precompartida del semaforo
  * @param int size: Tamaño del semaforo
  * @return int: Error si error. 0 si se ha creado, 1 si ya estaba creado
  * @return int* semid: identificador del semaforo creado
*/
int Crear_Semaforo(key_t key, int size, int *semid);

/**
  * @funcion Borrar_Semaforo
  * @date 2016/03/26
  * @brief borra un semaforo.
  * @param int semid: Identificador del semaforo.
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int Borrar_Semaforo(int semid);

/**
  * @funcion Inicializar_Semaforo
  * @date 2016/03/26
  * @brief Inicializa los semaforos indicados.
  * @param int semid: Identificador del semaforo.
  * @param unsigned short *array: Valores iniciales.
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int Inicializar_Semaforo(int semid, unsigned short* array);

/**
  * @funcion Down_Semaforo
  * @date 2016/03/26
  * @brief Baja el semaforo indicado
  * @param int semid: Identificador del semaforo.
  * @param int num_sem: Semaforo dentro del array.
  * @param int undo: Flag de modo persistente pese a finalización abrupta.
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int Down_Semaforo(int semid, int num_sem, int undo);

/**
  * @funcion DownMultiple_Semaforo
  * @date 2016/03/26
  * @brief Baja todos los semaforos del array indicado por active
  * @param int semid: Identificador del semaforo.
  * @param int size: Numero de semaforos dentro del array.
  * @param int undo: Flag de modo persistente pese a finalización abrupta.
  * @param int* active: Semaforos involucrados
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int DownMultiple_Semaforo(int semid, int size, int undo, int* active);

/**
  * @funcion Up_Semaforo
  * @date 2016/03/26
  * @brief Sube el semaforo indicado
  * @param int semid: Identificador del semaforo.
  * @param int num_sem: Semaforo dentro del array.
  * @param int undo: Flag de modo persistente pese a finalización abrupta.
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int Up_Semaforo(int id, int num_sem, int undo);

/**
  * @funcion UpMultiple_Semaforo
  * @date 2016/03/26
  * @brief Sube todos los semaforos del array indicado por active
  * @param int semid: Identificador del semaforo.
  * @param int size: Numero de semaforos dentro del array.
  * @param int undo: Flag de modo persistente pese a finalización abrupta.
  * @param int* active: Semaforos involucrados
  * @return int: OK si todo fue correcto, ERROR en caso de error.
*/
int UpMultiple_Semaforo(int id, int size,int undo, int* active);


#endif
