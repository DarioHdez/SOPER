/**
 * @file ejercicio5.c 
 * @brief Módulo de pruebas para semáforos

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/03/27
*/


#include "semaforos.h"

#define SEMKEY 75770
#define N_SEMAFOROS 2


int main() {
  
  int semid;

  /* Para operaciones up y down sobre semáforos */
  // int* maybe;
  
  /**Creamos una lista o conjunto con dos semáforos*/
  if (Crear_Semaforo(SEMKEY,N_SEMAFOROS, &semid) == ERROR) {
    return 1;
  }	

  if (Borrar_Semaforo(semid) == -1) {
    return 1;
  }

  /*Llamada a la inicialización, no estoy seguro de como se hace, stop*/
  return 0;
}
