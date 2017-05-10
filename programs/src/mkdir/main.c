#include "lib.h"

int main(char *args) {
    if (!*args) {
        fprintf(STDERR, "mkdir: Missing operand");
        exit(EXIT_FAILURE);
    }
    int res = mkdir(args, 0);
    if (res != 0) {
        fprintf(STDERR, "mkdir: Cannot create directory '%s': %s", args, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return 0;
}