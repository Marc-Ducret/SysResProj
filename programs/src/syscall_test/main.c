#include "lib.h"

int main() {
    char buffer[11];
    int nb = -1;
    memset(buffer, 0, 11);
    while (1) {
        sleep(20);
        nb = wait_channel(STDIN, 0);
        if (nb == -1)
            printf("Error : %s\n", strerror(errno));
        printf("Announced : %d\n", nb);
        int read = receive(STDIN, (u8*)buffer, 10);
        printf("Received : %d characters : <%s>\n", read, buffer);
    }
}
