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


 /*Este main creara los semaforos, intentara bajar los dos cuando esten a 0 y se bloqueara*/


int main() {
  
  int semid;
  unsigned short* array = (unsigned short *)malloc(sizeof(short)*N_SEMAFOROS);
  int *active = (int*)malloc(sizeof(int)*N_SEMAFOROS);

  /*Creamos una lista o conjunto con dos semáforos*/
  if (Crear_Semaforo(SEMKEY,N_SEMAFOROS, &semid) == ERROR) {
    printf("Error while creating sem\n");
    return 1;
  }

  array[0] = array[1] = 0;
  /*Inicalizo los semaforos a 1*/
  if (Inicializar_Semaforo(semid, array) == ERROR) {
    printf("Error while initializing sem\n");
    return 1;
  }

  /*Pruebo a subir el semaforo 1
  if (Up_Semaforo(semid,1,1) == ERROR) {
    printf("Error while operating sem 1 \n");
    return 1;
  }  

  active[0] = active[1] = 1;

  if (UpMultiple_Semaforo(semid,N_SEMAFOROS,1,active) == ERROR){
    printf("Error while operating Multsem up\n");
    return 1;
  }
  */

  printf("Me voy a bloquear hasta que ejecutes ejercicio5b\n");

  active[0] = active[1] = 1; /*Para indicar que quiero operar en los 2 semaforos*/

  /*if (Down_Semaforo(semid,1,1) == ERROR) {
    printf("Error while operating sem 1\n");
    return 1;
  } */ 

  if (DownMultiple_Semaforo(semid,N_SEMAFOROS,1,active) == ERROR){
    printf("Error while operating MultsemDown\n");
    return 1;
  }

  printf("Me he desbloqueado\n");

  free(array);
  free(active);

  if (Borrar_Semaforo(semid) == -1) { 
    printf("Error while deleting sem\n");
    return 1;
  }  

  return 0;
}
