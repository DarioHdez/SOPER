/**
 * @file cat.c
 * @brief Implementacion de un clon basico de la utilidad cat de UNIX
 * 
 * @author Dario Adrian Barroso
 * @author Angel Manuel Martin Canto
 * @date 2016-02-09
 */

#include <fcntl.h>
#include <unistd.h>

int loro(int in_fd, int out_fd) {
    char buffer[1024];
    int r;

    do {
        r = read(in_fd, buffer, 1024);
        if(r == -1)
            return 1;

        write(out_fd, buffer, r);
    } while(r);

    return 0;
}

int main(int argc, char* argv[]) {
    if(argc <= 1) {
        return loro(0, 1);
    } else {
        int argn;

        for(argn = 1; argn < argc; ++argn) {
            int fd = open(argv[argn], O_RDONLY);

            if(fd == -1)
                continue;

            loro(fd, 1);

            if(close(fd) == -1)
                return 1;
        }
    }

    return 0;
}
