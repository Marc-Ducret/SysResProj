#include "lib.h"
#include "parsing.h"

int aux_mv(char *src, char *dest) {
    // This uses fcopy, so it is very unefficient.
    int res = fcopy(src, dest);
    if (res == -1)
        return -1;
    
    res = remove(src);
    if (res == -1) {
        // Big problem : copied but not removed other !
        // Should check at start the rights and so on to be sure !
        int err = errno;
        fprintf(STDERR, "Created the new file, but couldn't remove the old one.\n");
        errno = err;
        return -1;
    }
    return 0;
}

int main(char *args) {
    char *paths[2];
    int nb = get_args(args, paths, 2);
    if (nb == -1)
        too_many_args("mv");
    
    if (nb == 0) {
        fprintf(STDERR, "mv: Missing operands");
        exit(EXIT_FAILURE);
    }
    if (nb == 1) {
        fprintf(STDERR, "mv: Missing destination operand");
        exit(EXIT_FAILURE);
    }
    char *src = paths[0];
    char *dest = paths[1];
    
    int res = aux_mv(src, dest);
    if (res != 0) {
        fprintf(STDERR, "mv: Cannot rename %s into %s: %s", src, dest, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return 0;
}