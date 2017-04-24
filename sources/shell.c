#include "shell.h"
#include "printing.h"
#include "keyboard.h"
#include "keycode.h"
#include "lib.h"
#include "kernel.h"
#include "syscall.h"

char cmd[CMD_SIZE];
int pos;
int run;
void newCmd() {
    kprintf("> ");
}

void execCmd() {
    if(     strEqual(cmd, "help")) kprintf("Available commands: help, clear, exit, int, test, launch\n");
    else if(strEqual(cmd, "exit")) run = 0;
    else if(strEqual(cmd, "clear")) {
        u8 color = make_color(COLOR_GREEN, COLOR_BLACK);
        clear(color);
        terminal_setcolor(color);
    } else if(strEqual(cmd, "test")) {
        for(int i = 0; i < 100; i ++)
            kprintf("%d\n", i);
    } else if(strEqual(cmd, "int")) {
        asm volatile ("int $0x03");
        asm volatile ("int $0x03");
    } else if(strEqual(cmd, "launch")) {
        launch();
    } else if(strEqual(cmd, "newlaunch")) {
        new_launch();
    } else if(strEqual(cmd, "mkdir")) {
        u8 mode = 0;
        mkdir("test1", mode);
    } else if(strEqual(cmd, "rmdir")) {
        rmdir("test1");
    }
    else kprintf("Unknown command (%s)\n", cmd);
    while(pos > 0)
        cmd[--pos] = 0;
    newCmd();
}

void keyTyped(int key) {
    if(key == KEY_SHIFT) scrollup();
    if(key == KEY_CTRL)  scrolldown();
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
            asm("hlt");
    }
}
