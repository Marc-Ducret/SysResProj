#include "keycode.h"

static char chars[128];

void initCharTable() {
    chars[KEY_A] = 'a';
    chars[KEY_B] = 'b';
    chars[KEY_C] = 'c';
    chars[KEY_D] = 'd';
    chars[KEY_E] = 'e';
    chars[KEY_F] = 'f';
    chars[KEY_G] = 'g';
    chars[KEY_H] = 'h';
    chars[KEY_I] = 'i';
    chars[KEY_J] = 'j';
    chars[KEY_K] = 'k';
    chars[KEY_L] = 'l';
    chars[KEY_M] = 'm';
    chars[KEY_N] = 'n';
    chars[KEY_O] = 'o';
    chars[KEY_P] = 'p';
    chars[KEY_Q] = 'q';
    chars[KEY_R] = 'r';
    chars[KEY_S] = 's';
    chars[KEY_T] = 't';
    chars[KEY_U] = 'u';
    chars[KEY_V] = 'v';
    chars[KEY_W] = 'w';
    chars[KEY_X] = 'x';
    chars[KEY_Y] = 'y';
    chars[KEY_Z] = 'z';
    chars[KEY_SPACE] = ' ';
    chars[KEY_ENTER] = '\n';
}

char getKeyChar(short key) {
    return chars[key];
}
