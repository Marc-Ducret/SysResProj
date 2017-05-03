#include "lib.h"


int main() {
    initCharTable();
    int chanid = knew_channel();
    char buffer[10];
    for (int i = 0; i < 10; i++)
        buffer[i] = '0' + i;
    set_char_at('a', COLOR_WHITE, COLOR_BLACK, 0, 0);
    int res = ksend(chanid, (void*) buffer, 10);
    set_char_at('b', COLOR_BLUE, COLOR_BLACK, 10, 10);
    for(;;)
        asm("hlt");
}