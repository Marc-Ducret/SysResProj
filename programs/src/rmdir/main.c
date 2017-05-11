#include "lib.h"

int main(char *args) {
    if (!*args) {
        fprintf(STDERR, "rmdir: Missing operand");
        exit(EXIT_FAILURE);
    }
    int res = rmdir(args);
    if (res != 0) {
        fprintf(STDERR, "rmdir: Cannot remove directory '%s': %s", args, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return 0;
}