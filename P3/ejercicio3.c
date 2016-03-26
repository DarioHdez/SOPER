#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

#define SEMKEY 75798
#define N_SEMAFOROS 2

int main (){

	/**	Declaración de variables*/
	int semid;  						/* ID de la lista de semáforos */
	struct sembuf sem_oper;  

	/* Para operaciones up y down sobre semáforos */
	union semun {
		int val;
		struct semid_ds *semstat;
		unsigned short *array;
	}arg;
	
	/**Creamos una lista o conjunto con dos semáforos*/
	semid = semget(SEMKEY, N_SEMAFOROS, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);

	if ((semid == -1) && errno == EEXIST)
		semid = semget(SEMKEY,N_SEMAFOROS ,SHM_R|SHM_W);
	
	if (semid == -1){
		perror("semget");
		exit(errno);
	}

	/**Inicializamos los semáforos*/
	arg.array = (unsigned short *)malloc(sizeof(short)*N_SEMAFOROS);

	arg.array[0] = arg.array[1] = 1;
	semctl(semid, N_SEMAFOROS, SETALL, arg); 
	
	/**Operamos sobre los semáforos */
	
	sem_oper.sem_num = 0; 				/* Actuamos sobre el semáforo 0 de la lista */
	sem_oper.sem_op = -1;  				/* Decrementar en 1 el valor del semáforo */
	sem_oper.sem_flg = SEM_UNDO;	/* Para evitar interbloqueos si un proceso acaba inesperadamente */
	
	semop (semid, &sem_oper, 1);
	
	sem_oper.sem_num = 1;					/* Actuamos sobre el semáforo 1 de la lista */
	sem_oper.sem_op = 1;					/* Incrementar en 1 el valor del semáforo */
	sem_oper.sem_flg = SEM_UNDO;	/* No es necesario porque ya se ha hecho anteriormente */
	semop (semid, &sem_oper, 1);

	/** Veamos los valores de los semáforos*/
	semctl (semid, N_SEMAFOROS, GETALL, arg);

	printf("Los valores de los semaforos son %d y %d\n", arg.array[0], arg.array[1]);

	/* Eliminar la lista de semáforos */
	semctl (semid, N_SEMAFOROS, IPC_RMID, 0);
	
	free(arg.array);

	return 0;
}
/* fin de la función main */	
