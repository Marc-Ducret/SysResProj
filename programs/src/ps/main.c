#include "lib.h"
#include "parsing.h"

char *state_to_str(p_state_t s) {
    switch (s) {
        case P_FREE:
            return "     Free      ";

        case P_BLOCKEDWRITING:
            return "Blocked Writing";

        case P_BLOCKEDREADING:
            return "Blocked Reading";

        case P_WAITING:
            return "    Waiting    ";

        case P_RUNNABLE:
            return "   Runnable    ";

        case P_ZOMBIE:
            return "    Zombie     ";
           
        case P_SLEEPING:
            return "   Sleeping    ";
    }
    return         "    Unknown    ";
}

char *pad(int a) {
    if (a < 10)
        return "  ";
    if (a < 100)
        return " ";
    if (a < 1000)
        return "";
    return "";
}

int main(char *args) {
    if (*args) {
        too_many_args("ps");
    }
    process_info_t p;
    pid_t pid = 0;
    printf("Existing processes:\n");
    printf("PID   PARENT       STATE             NAME");
    int res = pinfo(pid, &p);
    while (res != -1) {
        if (p.p_state != P_FREE)
            printf("\n %d%s    %d%s   %s    %s", pid, pad(pid), p.parent, pad(p.parent), state_to_str(p.p_state), p.name);
        pid++;
        res = pinfo(pid, &p);
    }
    exit(EXIT_SUCCESS);
    return 0;
}