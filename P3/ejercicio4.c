/**
 * @file ejercicio4.c 
 * @brief Implementa los semaforos

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/03/26
*/

#include "semaforos.h"


int Inicializar_Semaforo(int semid, unsigned short* array){
  if (semid == -1){
		perror("semget");
		exit(errno);
	}	
	


}

