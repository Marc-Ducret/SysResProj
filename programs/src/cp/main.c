#include "lib.h"
#include "parsing.h"

int main(char *args) {
    char *paths[2];
    int nb = get_args(args, paths, 2);
    if (nb == -1)
        too_many_args("cp");
    
    if (nb == 0) {
        fprintf(STDERR, "cp: Missing operands\n");
        exit(EXIT_FAILURE);
    }
    if (nb == 1) {
        fprintf(STDERR, "cp: Missing destination operand\n");
        exit(EXIT_FAILURE);
    }
    char *src = paths[0];
    char *dest = paths[1];
    
    int res = fcopy(src, dest);
    if (res != 0) {
        fprintf(STDERR, "cp: Cannot copy %s to %s: %s\n", src, dest, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return 0;
}