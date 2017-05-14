#include "lib.h"

void main() {
    int buf;
    for(;;) {
        wait(&buf);
        sleep(50);
    }
}
