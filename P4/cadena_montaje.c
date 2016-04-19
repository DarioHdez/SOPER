/**
  *
  * @author Ángel Manuel Martín 
  * @author Darío Adrián Hernández Barroso
  *
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

#define N 33


int procesoB(){



	exit(0);
}

int procesoC(){



  exit(0);
}

int main(int argc, char** argv) {

  if (argc != 3){
    printf("Use: ./cadena_montaje <read_file> <write_file >");
  }

  

  int pid;

  pid = fork();
  if (pid == 0){
    procesoB();
    return 0;
  }

  if (pid != 0)
    pid = fork();
  
  if (pid == 0){
    procesoC();
    return 0;
  }  

  wait(pid)
	return 0;
}