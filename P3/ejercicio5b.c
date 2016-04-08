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

/*Este main complementara al ejercicio5.c, y subira los semaforos para desbloquear al otro*/

 int main() {

  int semid;
  int *active = (int*)malloc(sizeof(int)*N_SEMAFOROS);

  /*Nos unimos a los semaforos ya creados usando la misma SEMKEY*/
  if (Crear_Semaforo(SEMKEY,N_SEMAFOROS, &semid) == ERROR) {
    printf("Error while creating sem\n");
    return 1;
  }

  active[0] = active[1] = 1;

  printf("Yo desbloqueo ejercicio5\n");

  active[0] = active[1] = 1;

  if (UpMultiple_Semaforo(semid,N_SEMAFOROS,1,active) == ERROR){
    printf("Error while operating Multsem up\n");
 	  return 1;
  }	

  free(active);

  return 0;
 }