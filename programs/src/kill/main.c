#include "lib.h"
#include "parsing.h"

int main(char *args) {
    char *pid_string;
    pid_t pid;
    int nb_args = get_args(args, &pid_string, 1);

    if (nb_args == -1) {
        too_many_args("kill");
        return 0;
    }
    if (nb_args == 0) {
        fprintf(STDERR, "kill: Missing poor target process");
        exit(EXIT_FAILURE);
    }
    if (nb_args == 1) {
        errno = EINVAL;
        pid = string_to_int(pid_string);
        if (pid == -1 || kill(pid) == -1) {
            fprintf(STDERR, "kill: Cannot kill process %s: %s", pid_string, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
    return 0;
}