#include "lib.h"
#include "parsing.h"

int main(char *args) {
    char *path;
    int nb_args = get_args(args, &path, 1);
    if (nb_args == -1) {
        too_many_args("cat");
    }
    if (nb_args == 0) {
        char buffer[512];
        int nb;
        int run = 1;
        int written;
        while (run) {
            nb = wait_channel(STDIN, 0);
            if (nb < 0) {
                fprintf(STDERR, "cat: Cannot read input: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            nb = receive(STDIN, buffer, 512);
            if (nb < 0) {
                fprintf(STDERR, "cat: Cannot read input: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            for (int j = 0; j < nb; j++) {
                if (buffer[j] == TERMINATION_KEY) {
                    run = 0;
                    nb = j;
                }
            }
            while (nb > 0) {
                written = wait_channel(STDOUT, 1);
                if (written < 0) {
                    fprintf(STDERR, "cat: Cannot write to output: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                written = send(STDOUT, buffer, nb);
                if (written < 0) {
                    fprintf(STDERR, "cat: Cannot write to output: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                nb -= written;
            }
        }
        flush(STDOUT);
    }
    if (nb_args == 1) {
        fd_t fd = fopen(path, O_RDONLY);
        if (fd < 0) {
            fprintf(STDERR, "cat: Couldn't open %s: %s\n", path, strerror(errno));
            exit(EXIT_FAILURE);
        }
        char buffer[512];
        int nb;
        int written;
        while (1) {
            nb = read(fd, buffer, 512);
            if (nb == 0) {
                close(fd);
                exit(EXIT_SUCCESS);
            }
            if (nb == -1) {
                fprintf(STDERR, "cat: Failed to read file %s: %s\n", path, strerror(errno));
                close(fd);
                exit(EXIT_FAILURE);
            }
            while (nb > 0) {
                written = wait_channel(STDOUT, 1);
                if (written < 0) {
                    fprintf(STDERR, "cat: Cannot write to output: %s\n", strerror(errno));
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                written = send(STDOUT, buffer, nb);
                if (written < 0) {
                    fprintf(STDERR, "cat: Cannot write to output: %s\n", strerror(errno));
                    close(fd);
                    exit(EXIT_FAILURE);
                }
                nb -= written;
            }
        }
    }
    exit(EXIT_SUCCESS);
    return 0;
}