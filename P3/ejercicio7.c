/**
 * @file ejercicio7.c 
 * @brief Implementacion del puente con hilos

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/04/06
*/

#include "semaforos.h"

#define SEMKEY 66666

int flujo_de_coches;

void coche(int semid, int sema_dir, int sema_glo, int sentido) {
	sleep(rand() % 10);


	Down_Semaforo(semid,sema_dir, 1):
		
	if (flujo_de_coches == 0)
		Down_Semaforo(semid,sema_glo, 1);

	flujo_de_coches += sentido;

	Up_Semaforo(semid,sema_dir,1);

	sleep(1);

	Down_Semaforo(semid,sema_dir,1);

	flujo_de_coches -= sentido;

	if (flujo_de_coches == 0)
		Up_Semaforo(semid,sema_glo,1);

	Up_Semaforo(semid,sema_dir,1);
}

int main() {
	int semid;

    if (Crear_Semaforo(SEMKEY, 2, &semid) == ERROR) {
        return 1;
    }   

    flujo_de_coches = 0;

    /*Hay que crear los hilosmiau*/


    if (Borrar_Semaforo(semid) == -1) {
        return 1;
    }

    return 0;
}