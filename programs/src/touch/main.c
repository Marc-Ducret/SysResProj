#include "lib.h"
#include "parsing.h"

int main(char *args) {
    char *file;
    int nb = get_args(args, &file, 1);
    if (nb == -1)
        too_many_args("touch");
    
    if (nb == 0) {
        fprintf(STDERR, "touch: Missing operand");
        exit(EXIT_FAILURE);
    }
    
    fd_t res = fopen(file, O_CREAT);
    
    if (res == -1) {
        fprintf(STDERR, "touch: Couldn't touch file %s: %s", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(res);
    exit(EXIT_SUCCESS);
    return 0;
}