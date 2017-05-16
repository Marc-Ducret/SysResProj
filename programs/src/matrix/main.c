#include "lib.h"

#define NB_POSSIBLES 26
char possibles[NB_POSSIBLES];

int main(char *args) {
    for (int i =0; i < 26; i++) {
        possibles[i] = 'a' + i;
    }
    int activated[VGA_WIDTH];
    u8 colors[VGA_WIDTH];
    memset(colors, 0, VGA_WIDTH);
    int period = 25;
    while (1) {
        sleep(100);
        for (int i = 0; i < VGA_WIDTH; i++) {
            if (!(rand() % period)) {
                activated[i] = rand()%2;
                colors[i] = rand()%16;
            }
            if (activated[i]) {
                printf("%fg%c%pfg", colors[i], 30 + (rand() % 220));//, colors[i], possibles[rand() % NB_POSSIBLES]);
            }
            else
                printf(" ");//, BLACK);
        }
    }
}