#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_MATRIX 2

typedef struct Datos_ {
	int id;
	int f;
	int mul;
	int* mat;
} Datos;

Datos datos[NUM_MATRIX];

void* multiplicador(void* ptr) {
	Datos *d = (Datos*)ptr;
	int otro = (d->id == 1) ? 0 : 1;

	int c;

	for(d->f = 0; d->f < 3; ++(d->f)) {
		for(c = 0; c < 3; ++c) {
			d->mat[d->f*3+c] *= d->mul;
		}

		printf("Hilo %d multiplicando fila %d resultado %d %d %d\n"
				"el Hilo %d va por la fila %d\n",
				d->id, d->f, d->mat[d->f*3], d->mat[d->f*3+1], d->mat[d->f*3+2],
				otro, datos[otro].f);

		sleep(1);
	}

	return ptr;
}

int main() {
	int m;
	int mat[NUM_MATRIX][9];
	pthread_t th[NUM_MATRIX];

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
		if(pthread_create(&th[m], NULL, multiplicador, &datos[m]) != 0) {
			exit(1);
		}
	}

	for(m = 0; m < NUM_MATRIX; ++m) {
		pthread_join(th[m], NULL);
	}

	return 0;
}