/**
 * @file cadena_montaje.c
 * @brief Transforma un fichero a mayusculas usando tres procesos comunicados por mensajes.

 * @author Darío Adrián Hernández
 * @author Ángel Manuel Martín
 * @date 2016/04/29
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FILEKEY "/bin/cat"
#define KEY1 1234
#define KEY2 1235

#define PERMS 0600

#define BLOCK_SIZE 300

key_t key1;
key_t key2;

int check_err(const char *fname, int val) {
    if (val == -1) {
        char err_desc[256];
        if (strerror_r(errno, err_desc, sizeof(err_desc))) {
            fprintf(stderr, "PID:%d :: Fallo de %s: Error obteniendo descripcion de error\n", getpid(), fname);
            exit(1);
        }
        fprintf(stderr, "PID:%d :: Fallo de %s: errno = %s\n", getpid(), fname, err_desc);
        exit(1);
    }

    return val;
}

void* check_mem(void* mem) {
    if (mem == NULL) {
        fprintf(stderr, "Fallo en malloc\n");
        exit(1);
    }

    return mem;
}

int procesoB() {
    int msq1_id = check_err("msgget", msgget(key1, 0));
    int msq2_id = check_err("msgget", msgget(key2, 0));
    char *buf = (char*)check_mem(malloc(sizeof(long) + BLOCK_SIZE));
    ssize_t i;
    ssize_t bytes_read;

    printf("Proceso B PID = %d\n", getpid());

    do {
        bytes_read = check_err("msgrcv(msq1_id, buf, BLOCK_SIZE, 0, 0)",
                               msgrcv(msq1_id, buf, BLOCK_SIZE, 0, 0));

        for (i = 0; i < bytes_read; ++i) {
            buf[sizeof(long) + i] = (char)toupper(buf[sizeof(long) + i]);
        }

        printf("Procesados %zi bytes\n", bytes_read);

        *((long*)buf) = 1;
        check_err("msgsnd(msq2_id, buf, bytes_read, 0)", msgsnd(msq2_id, buf, bytes_read, 0));
    } while(bytes_read);

    free(buf);

    return 0;
}

int procesoC(char *write_file) {
    int msq2_id = check_err("msgget", msgget(key2, 0));
    int fd = check_err("open(write_file, O_CREAT | O_RDWR)", open(write_file, O_CREAT | O_RDWR));
    char *buf = (char*)check_mem(malloc(BLOCK_SIZE));
    ssize_t bytes_read;

    printf("Proceso C PID = %d\n", getpid());

    do {
        bytes_read = check_err("msgrcv(msq2_id, buf, BLOCK_SIZE, 0, 0)",
                               msgrcv(msq2_id, buf, BLOCK_SIZE, 0, 0));

        write(fd, buf + sizeof(long), bytes_read);
    } while(bytes_read);

    free(buf);
    close(fd);

    return 0;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: ./cadena_montaje <read_file> <write_file>\n");
        return 1;
    }

    char *read_file = argv[1];
    char *write_file = argv[2];

    key1 = check_err("ftok(FILEKEY, KEY1)", ftok(FILEKEY, KEY1));
    key2 = check_err("ftok(FILEKEY, KEY2)", ftok(FILEKEY, KEY2));

    int msq1_id = check_err("msgget", msgget(key1, IPC_CREAT | PERMS));
    int msq2_id = check_err("msgget", msgget(key2, IPC_CREAT | PERMS));

    int pid;

    pid = check_err("fork", fork());
    if (pid == 0) {
        exit(procesoB());
    }

    pid = check_err("fork", fork());
    if (pid == 0) {
        exit(procesoC(write_file));
    }

    // Proceso A

    printf("Proceso A PID = %d\n", getpid());

    int fd = check_err("open(read_file, O_RDONLY)", open(read_file, O_RDONLY));
    char *buf = (char*)check_mem(malloc(sizeof(long) + BLOCK_SIZE));

    int bytes_read = 0;
    do {
        bytes_read = check_err("read(fd, buf + sizeof(long), BLOCK_SIZE)", read(fd, buf + sizeof(long), BLOCK_SIZE));
        *((long*)buf) = 1;
        check_err("msgsnd(msq1_id, buf, bytes_read, 0)", msgsnd(msq1_id, buf, bytes_read, 0));
    } while (bytes_read > 0);

    free(buf);
    check_err("close(fd)", close(fd));

    wait(NULL);
    wait(NULL);

    msgctl(msq1_id, IPC_RMID, NULL);
    msgctl(msq2_id, IPC_RMID, NULL);

    return 0;
}
