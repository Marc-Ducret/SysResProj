#include "lib.h"

int main(char *args) {
    if (!*args) {
        fprintf(STDERR, "rm: Missing operand\n");
        exit(EXIT_FAILURE);
    }
    int res = remove(args);
    if (res != 0) {
        fprintf(STDERR, "rm: Cannot delete '%s': %s\n", args, strerror(errno));
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    return 0;
}