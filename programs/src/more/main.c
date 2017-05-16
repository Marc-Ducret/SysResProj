#include "lib.h"
#include "parsing.h"

volatile int quota = 0;
int wait_key() {
    int nb = wait_channel(STDIN, 0);
    if (nb == -1) {
        printf("Cannot read input: %s\n", strerror(errno));
        return -1;
    }
    char buffer[512];
    nb = receive(STDIN, buffer, 512);
    for (int j = 0; j < nb; j++) {
        if (buffer[j] == TERMINATION_KEY) {
            return -1;
        }
        else
            quota++;
    }
    return 0;
}

int main(char *args) {
    char *path;
    int nb_args = get_args(args, &path, 1);
    if (nb_args == -1) {
        too_many_args("more");
    }
    if (nb_args == 0) {
        fprintf(STDERR, "more: Missing file argument\n");
        exit(EXIT_FAILURE);
    }
    if (nb_args == 1) {
        fd_t fd = fopen(path, O_RDONLY);
        if (fd < 0) {
            fprintf(STDERR, "more: Couldn't open %s: %s\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        char buffer[512];
        int nb;
        int written;
        int run = 1;
        quota = 24;
        int this_time = 0;
        char *start = 0;
        while (run) {
            nb = read(fd, buffer, 512);
            if (nb == 0) {
                close(fd);
                exit(EXIT_SUCCESS);
            }
            if (nb == -1) {
                fprintf(STDERR, "more: Failed to read file %s: %s\n", path, strerror(errno));
                close(fd);
                exit(EXIT_FAILURE);
            }
            start = buffer;
            while (nb > 0) {
                while (run && !quota) {
                    run = (wait_key() != -1);
                }
                if (!run)
                    break;
                
                this_time = 0;
                while(this_time < nb && start[this_time] != '\n')
                    this_time++;
                
                if (this_time < nb && start[this_time] == '\n') {
                    quota--;
                    this_time++;
                }
                
                written = wait_channel(STDOUT, 1);
                if (written < 0) {
                    fprintf(STDERR, "more: Cannot write to output: %s\n", strerror(errno));
                    close(fd);
                    flush(STDOUT);
                    exit(EXIT_FAILURE);
                }
                written = send(STDOUT, start, this_time);
                if (written < 0) {
                    fprintf(STDERR, "more: Cannot write to output: %s\n", strerror(errno));
                    flush(STDOUT);
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                nb -= written;
                start += written;
            }
            if (!run)
                break;
        }
    }
    flush(STDOUT);
    exit(EXIT_SUCCESS);
    return 0;
}