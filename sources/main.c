#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "printing.h"
#include "io.h"
#include "gdt.h"
#include "keycode.h"
#include "shell.h"
#include "timer.h"
#include "paging.h"

void dieSlowly() {
    clear(COLOR_LIGHT_GREEN);
    terminal_setcolor(make_color(COLOR_LIGHT_RED, COLOR_LIGHT_GREEN));
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) 
        putchar(i % 2 == 0 ? ' ' : '#');
    asm("cli");
    for(;;) asm("hlt");
}

void kmain() {
    terminal_initialize();
    initCharTable();
    kprintf("Init\n");
    init_gdt();
    init_idt();
    init_pic();
    init_timer(100);
    init_paging(0x1000000);
    asm("sti");
    shell();
    dieSlowly();
}
