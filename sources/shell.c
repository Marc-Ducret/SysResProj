#include "shell.h"
#include "printing.h"
#include "keyboard.h"
#include "keycode.h"
#include "lib.h"

char cmd[CMD_SIZE];
int pos;
int run;
void newCmd() {
    kprintf("> ");
}

void execCmd() {
    if(     strEqual(cmd, "help")) kprintf("Available commands: help, clear, exit, test\n");
    else if(strEqual(cmd, "exit")) run = 0;
    else if(strEqual(cmd, "clear")) {
        u8 color = make_color(COLOR_GREEN, COLOR_BLACK);
        clear(COLOR_BLACK);
        terminal_setcolor(color);
    } else if(strEqual(cmd, "test")) {
        asm volatile ("int $3");
    }
    else kprintf("Unknown command (%s)\n", cmd);
    while(pos > 0)
        cmd[--pos] = 0;
    newCmd();
}

void keyTyped(int key) {
	char c = getKeyChar(key);
    if(c == '\n') {
        putchar('\n');
        execCmd();
    } else if(c != 0 && pos < CMD_SIZE) {
        putchar(c);
        cmd[pos++] = c;
    } else if(key == KEY_BACKSPACE && pos > 0) {
        erase();
        cmd[--pos] = 0;
    }
}

void shell() {
    run = 1;
    newCmd();
    while(run) {
        int event;
        while((event = nextKeyEvent()) >= 0)
		    if(event <= 0x80) {
		    	keyTyped(event);
		    }
    }
}
