#include "shell.h"
#include "printing.h"
#include "keyboard.h"
#include "keycode.h"

char cmd[CMD_SIZE];
int pos;

void newCmd() {
    kprintf("\n> ");
}

void execCmd() {
    kprintf("\n execute (%s)", cmd);
    while(pos > 0)
        cmd[--pos] = 0;
    newCmd();
}

void shell() {
    newCmd();
    while(1) {
        int key = waitPress();
        char c = getKeyChar(key);
        if(c == '\n') execCmd();
        else if(c != 0 && pos < CMD_SIZE) {
            putchar(c);
            cmd[pos++] = c;
        } else if(key == KEY_BACKSPACE && pos > 0) {
            erase();
            cmd[--pos] = 0;
        }
    }
}
