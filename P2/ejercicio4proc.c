/**
 * @file ejercicio4proc.c
 * @brief Ejercicio 4: Multiplica 2 matrices de 3x3 en 2 procesos
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-3-09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define NUM_MATRIX 2

typedef struct Datos_ {
	int id;
	int f;
	int mul;
	int* mat;
} Datos;

Datos datos[NUM_MATRIX];

void multiplicador(void* ptr) {
	Datos *d = (Datos*)ptr;

	int c;

	for(d->f = 0; d->f < 3; ++(d->f)) {
		for(c = 0; c < 3; ++c) {
			d->mat[d->f*3+c] *= d->mul;
		}

		printf("Proceso %d multiplicando fila %d resultado %d %d %d\n",
				d->id, d->f, d->mat[d->f*3], d->mat[d->f*3+1], d->mat[d->f*3+2]);

		sleep(1);
	}

	exit(0);
}

int main() {
	int pid;
	int m;
	int mat[NUM_MATRIX][9];

	for(m = 0; m < NUM_MATRIX; ++m) {
		printf("Introduzca multiplicador %d:\n", m + 1);
		scanf("%d", &datos[m].mul);
	}

	for(m = 0; m < NUM_MATRIX; ++m) {
		int i;

		printf("Introduzca matriz %d:\n", m + 1);

		for(i = 0; i < 9; ++i) {
			scanf("%d", &mat[m][i]);
		}

		datos[m].mat = mat[m]; 
	}

	for(m = 0; m < NUM_MATRIX; ++m) {
		datos[m].id = m;
		datos[m].f = 0;
		if((pid = fork()) == 0) {
			multiplicador(&datos[m]);
			exit(0);
		}
	}

	for(m = 0; m < NUM_MATRIX; ++m) {
		wait(0);
	}

	return 0;
}
