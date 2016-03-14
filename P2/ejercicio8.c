/**
 * @file ejercicio8.c
 * @brief Ejercicio 8: Recepción-Envío de señales consecutivos
 *
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-3-04
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

#define NUM_PROCESS 5

int raizpid;
int cpid;

void padre_usr1() {
	printf("Padre raiz recibe SIGUSR1\n");
	sleep(2);
	printf("Padre raiz envia SIGUSR2\n");
	kill(cpid, SIGUSR2);
}

void padre_usr2() {
	printf("Padre raiz recibe SIGUSR2\n");
	sleep(1);
	kill(cpid, SIGTERM);
	wait(0); //Para que no haya huerfanos
	exit(0);
}

void relay_usr1_up() {
	printf("Hijo@%d, recibe SIGUSR1\n", getpid());
	sleep(2);
	kill(getppid(), SIGUSR1);
}

void relay_usr2_down() {
	printf("Hijo@%d, recibe SIGUSR2\n", getpid());
	sleep(1);
	kill(cpid, SIGUSR2);
}

void relay_term_down() {
	printf("Hijo@%d, recibe SIGTERM\n", getpid());
	sleep(1);
	kill(cpid, SIGTERM);
	wait(0); //Para que no haya huerfanos
	exit(0);
}

void ultimo_hijo_usr2() {
	printf("Ultimo hijo recibe SIGUSR2\n");
	sleep(2);
	printf("Ultimo hijo envia SIGUSR2\n");
	kill(raizpid, SIGUSR2);
}

void ultimo_hijo_term() {
	printf("Ultimo hijo recibe SIGTERM\n");
	exit(0);
}

int main() {
	int p;
	pid_t pid;

	raizpid = getpid();

	pid = fork();

	if(pid) { //Padre raiz
		cpid = pid;
		signal(SIGUSR1, padre_usr1);
		signal(SIGUSR2, padre_usr2);
		pause(); //Esperar SIGUSR1
		pause(); //Esperar SIGUSR2
		exit(0);
	}

	for (p = 0; p < NUM_PROCESS - 1; ++p) {
		pid = fork();

		if (pid) { //Si es padre
			cpid = pid;
			signal(SIGUSR1, relay_usr1_up);
			signal(SIGUSR2, relay_usr2_down);
			signal(SIGTERM, relay_term_down);
			pause(); //Esperar SIGUSR1
			pause(); //Esperar SIGUSR2
			pause(); //Esperar SIGTERM
			exit(0);
		}

	}

	//Ultimo hijo
	signal(SIGUSR2, ultimo_hijo_usr2);
	signal(SIGTERM, ultimo_hijo_term);
	sleep(5);
	printf("Ultimo hijo envia SIGUSR1\n");
	kill(getppid(), SIGUSR1);
	pause(); //Esperar SIGUSR2
	pause(); //Esperar SIGTERM

	return 0;
}