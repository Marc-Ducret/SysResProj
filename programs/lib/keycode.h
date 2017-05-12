#ifndef KEYCODE_H
#define KEYCODE_H

#include "int.h"

#define KEY_ESCAPE 1
#define KEY_1 2
#define KEY_2 3
#define KEY_3 4
#define KEY_4 5
#define KEY_5 6
#define KEY_6 7
#define KEY_7 8
#define KEY_8 9
#define KEY_9 10
#define KEY_0 11
#define KEY_RPAR 12
#define KEY_EQUAL 13
#define KEY_BACKSPACE 14
#define KEY_TAB 15
#define KEY_A 16
#define KEY_Z 17
#define KEY_E 18
#define KEY_R 19
#define KEY_T 20
#define KEY_Y 21
#define KEY_U 22
#define KEY_I 23
#define KEY_O 24
#define KEY_P 25
#define KEY_HAT 26
#define KEY_DOLLAR 27
#define KEY_ENTER 28
#define KEY_CTRL 29
#define KEY_Q 30
#define KEY_S 31
#define KEY_D 32
#define KEY_F 33
#define KEY_G 34
#define KEY_H 35
#define KEY_J 36
#define KEY_K 37
#define KEY_L 38
#define KEY_M 39
#define KEY_PERCENT 40
#define KEY_SQUARE_SUPERSCRIPT 41
#define KEY_SHIFT 42
#define KEY_STAR 43
#define KEY_W 44
#define KEY_X 45
#define KEY_C 46
#define KEY_V 47
#define KEY_B 48
#define KEY_N 49
#define KEY_COMMA 50
#define KEY_SEMI_COLON 51
#define KEY_COLON 52
#define KEY_EXCLAMATION 53
#define KEY_RSHIFT 54
#define KEY_NUM_STAR 55
#define KEY_ALT_GR 56
#define KEY_SPACE 57
#define KEY_NUM_1 79
#define KEY_DOWN 80
#define KEY_NUM_3 81
#define KEY_LEFT 75
#define KEY_NUM_5 76
#define KEY_RIGHT 77
#define KEY_NUM_7 71
#define KEY_UP 72
#define KEY_NUM_9 73
#define KEY_NUM_0 82
// TODO ARROWS have same code ?! (ci dessus)
#define KEY_COMP 86
#define TERMINATION_KEY 0

#define CHAR_UP 0x11
#define CHAR_DOWN 0x12
#define CHAR_LEFT 0x13
#define CHAR_RIGHT 0x14
char getKeyChar(u8 key, u8 shift, u8 alt);
void initCharTable();

#endif
