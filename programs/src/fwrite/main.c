#include "lib.h"
#include "parsing.h"

int main(char *args) {
    args_t p;
    
    if (parse(args, &p) == -1 || p.nb_args > 1)
        too_many_args("fwrite");
    
    if (p.nb_args == 0) {
        fprintf(STDERR, "fwrite: Missing destination file argument\n");
        exit(EXIT_FAILURE);
    }
    int append = 0;
    for (int i = 0; i < p.nb_opts; i++) {
        if (strEqual(p.opts[i], "a"))
            append = 1;
        else {
            fprintf(STDERR, "fwrite: Unknown option -%s\n", p.opts[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    char *path = p.args[0];
    sid_t sid = create_stream(path, append);
    if (sid < 0) {
        fprintf(STDERR, "fwrite: Couldn't create a stream to %s: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    char buffer[512];
    int nb;
    int run = 1;
    int res;
    while (run) {
        nb = wait_channel(STDIN, 0);
        if (nb < 0) {
            if (errno == EALONE)
                exit(EXIT_SUCCESS);
            fprintf(STDERR, "fwrite: Cannot read input: %s\n", strerror(errno));
            close_stream(sid);
            exit(EXIT_FAILURE);
        }
        nb = receive(STDIN, buffer, 512);
        if (nb < 0) {
            fprintf(STDERR, "fwrite: Cannot read input: %s\n", strerror(errno));
            close_stream(sid);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < nb; j++) {
            if (buffer[j] == TERMINATION_KEY) {
                run = 0;
                break;
            }
            res = stream_putchar(buffer[j], sid);
            if (res == -1) {
                fprintf(STDERR, "fwrite: Cannot write to file %s: %s\n", path, strerror(errno));
                close_stream(sid);
                exit(EXIT_FAILURE);
            }
        }
    }
    close_stream(sid);
    exit(EXIT_SUCCESS);
    return 0;
}