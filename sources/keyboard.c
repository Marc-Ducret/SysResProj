#include "io.h"

int keyboardState() {
    return inportb(0x60);
}

int waitPress() {
    int cur = keyboardState();
    int key;
    while((key = keyboardState()) == cur || key >= 128) cur = key;
    return key;
}
