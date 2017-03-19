#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "printing.h"
#include "io.h"
#include "gdt.h"
#include "keycode.h"
#include "shell.h"

void dieSlowly() {
    clear(COLOR_LIGHT_GREEN);
    terminal_setcolor(make_color(COLOR_LIGHT_RED, COLOR_LIGHT_GREEN));
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) 
        putchar(i % 2 == 0 ? ' ' : '#');
    for(;;);
}

void kmain() {
	terminal_initialize();
	initCharTable();
    kprintf("Init\n");
    init_gdt();
    init_idt();
    init_pic();
    //asm("sti");
    //asm volatile ("int $0x3");
    shell();
    dieSlowly();
}
