/**
 * @file ejercicio6.c 
 * @brief 

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín 
 * @date 2016/04/06
*/

#include "semaforos.h"

#define SEMKEY 66666

int coche() {

}

int main() {
    int semid, id_zone;

    if (Crear_Semaforo(SEMKEY, 1, &semid) == ERROR) {
        return 1;
    }   



    if (Borrar_Semaforo(semid) == -1) {
        return 1;
    }

    return 0;
}