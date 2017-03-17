#include "shell.h"
#include "printing.h"
#include "keyboard.h"
#include "keycode.h"
#include "lib.h"

char cmd[CMD_SIZE];
int pos;
int run;
void newCmd() {
    kprintf("\n> ");
}

void execCmd() {
    if(strEqual(cmd, "exit")) run = 0;
    else kprintf("\nUnknown command (%s)", cmd);
    while(pos > 0)
        cmd[--pos] = 0;
    newCmd();
}

void shell() {
    run = 1;
    newCmd();
    while(run) {
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
