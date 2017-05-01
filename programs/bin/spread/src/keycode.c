#include "keycode.h"

static char chars[128];

void initCharTable() {
    for(int i = 0; i < 128; i++) chars[i] = 0;
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
    chars[KEY_1] = '1';
    chars[KEY_2] = '2';
    chars[KEY_3] = '3';
    chars[KEY_4] = '4';
    chars[KEY_5] = '5';
    chars[KEY_6] = '6';
    chars[KEY_7] = '7';
    chars[KEY_8] = '8';
    chars[KEY_9] = '9';
    chars[KEY_0] = '0';
    chars[KEY_RPAR] = ')';
    chars[KEY_PERCENT] = '%';
    //chars[KEY_SQUARE_SUPERSCRIPT] = '';
    //chars[KEY_SHIFT] = '';
    chars[KEY_STAR] = '*';
    chars[KEY_HAT] = '^';
    chars[KEY_DOLLAR] = '$';
    chars[KEY_COMMA] = ',';
    chars[KEY_SEMI_COLON] = ';';
    chars[KEY_COLON] = ':';
    chars[KEY_EXCLAMATION] = '!';
    chars[KEY_EQUAL] = '=';
    //chars[KEY_RSHIFT] = '';
    //chars[KEY_NUM_STAR] = '';
    //chars[KEY_ALT_GR] = '';
}

char getKeyChar(short key) {
    return chars[key];
}
