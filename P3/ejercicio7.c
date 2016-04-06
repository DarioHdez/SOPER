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

void coche(int sema_dir, int sema_glo, int sentido) {
	sleep(rand() % 10);


	down(sema_dir);
	if (flujo_de_coches == 0)
		down(sema_glo);

	flujo_de_coches += sentido;

	up(sema_dir);



	sleep(1);



	down(sema_dir);

	flujo_de_coches -= sentido;

	if (flujo_de_coches == 0)
		up(sema_glo);

	up(sema_dir);
}

int main() {
	int semid;

    if (Crear_Semaforo(SEMKEY, 2, &semid) == ERROR) {
        return 1;
    }   

    flujo_de_coches = 0;

    


    if (Borrar_Semaforo(semid) == -1) {
        return 1;
    }

    return 0;
}