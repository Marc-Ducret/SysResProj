#include "lib.h"
#include "parsing.h"

int kill_rec(pid_t pid) {
    // Kills the process and all its descendants.
    process_info_t p;
    int res = pinfo(pid, &p);
    if (res == -1)
        return -1;
    if (p.p_state == P_FREE || p.p_state == P_ZOMBIE) {
        errno = EINVAL;
        return -1;
    }
    
    for (pid_t child = 0; child < NUM_PROCESSES; child++) {
        int res = pinfo(child, &p);
        if (res == -1)
            continue;
        if (child != pid && p.parent == pid && p.p_state != P_FREE && p.p_state != P_ZOMBIE)
            res = kill_rec(p.pid);
        if (res == -1)
            return -1;
    }
    printf("Gonna kill %d\n", pid);
    return kill(pid);
}

int main(char *args) {
    char *pid_string;
    pid_t pid;
    int nb_args = get_args(args, &pid_string, 1);

    if (nb_args == -1) {
        too_many_args("kill");
        return 0;
    }
    if (nb_args == 0) {
        fprintf(STDERR, "kill: Missing poor target process\n");
        exit(EXIT_FAILURE);
    }
    if (nb_args == 1) {
        errno = EINVAL;
        pid = string_to_int(pid_string);
        if (pid == -1 || kill_rec(pid) == -1) {
            fprintf(STDERR, "kill: Cannot kill process %s: %s\n", pid_string, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
    return 0;
}