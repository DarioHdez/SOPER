#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void padre();
void hijo(int p_up[2]);
void nieto(int p_up[2]);

void padre() {
	int p[2][2];
	int i;
	int pid = getpid();
	int child[2];
	char buf[1024];

	printf("Padre@%d\n", pid);

	for(i = 0; i < 2; ++i) {
		int childpid;
		int ret = pipe(p[i]);

		if(ret == -1) {
			exit(EXIT_FAILURE);
		}

		if((childpid = fork()) == 0) { /*hijo*/
			hijo(p[i]);
		}

		child[i] = childpid;
	}

	for(i = 0; i < 2; ++i) {
		sprintf(buf, "Datos enviados a través de la tubería por el proceso PID=%d\n", pid);
		write(p[i][1], buf, strlen(buf));
		close(p[i][1]);
	}

	for(i = 0; i < 2; ++i) {
		int child_status;

		waitpid(child[i], &child_status, 0);
		printf("Padre:Hijo@%d terminado con %d\n", child[i], child_status);

		read(p[i][0], buf, 1024);
		printf("Padre:%s", buf);
		close(p[i][0]);
	}

	exit(0);

}

void hijo(int p_up[2]) {
	int p[2][2];
	int pid = getpid();
	int i;
	char buf[1024];
	int gchild[2];

	read(p_up[0], buf, 1024);
	printf("Hijo@%d:%s", pid, buf);
	close(p_up[0]);

	sprintf(buf, "Datos enviados a través de la tubería por el proceso PID=%d\n", pid);
	write(p_up[1], buf, strlen(buf));
	close(p_up[1]);

	for(i = 0; i < 2; ++i) {
		int gchildpid;
		int ret = pipe(p[i]);

		if(ret == -1) {
			exit(EXIT_FAILURE);
		}

		if((gchildpid = fork()) == 0) { /*nieto*/
			nieto(p[i]);
		}

		gchild[i] = gchildpid;
	}

	for(i = 0; i < 2; ++i) {
		sprintf(buf, "Datos enviados a través de la tubería por el proceso PID=%d\n", pid);
		write(p[i][1], buf, strlen(buf));
		close(p[i][1]);
	}

	for(i = 0; i < 2; ++i) {
		int gchild_status;

		waitpid(gchild[i], &gchild_status, 0);
		printf("Hijo:Nieto@%d terminado con %d\n", gchild[i], gchild_status);

		read(p[i][0], buf, 1024);
		printf("Hijo:%s", buf);
		close(p[i][0]);
	}

	exit(0);
}

void nieto(int p_up[2])  {

	int pid = getpid();
	char buf[1024];

	read(p_up[0], buf, 1024);
	printf("Nieto@%d:%s", pid, buf);
	close(p_up[0]);

	sprintf(buf, "Datos enviados a través de la tubería por el proceso PID=%d\n", pid);
	write(p_up[1], buf, strlen(buf));
	close(p_up[1]);

	exit(0);
}

int main() {
	padre();

	return 0;
}
