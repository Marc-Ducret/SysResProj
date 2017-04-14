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
#include "lib.h"
#include "multiboot.h"

void dieSlowly() {
    clear(make_color(COLOR_LIGHT_GREEN, COLOR_LIGHT_GREEN));
    terminal_setcolor(make_color(COLOR_LIGHT_RED, COLOR_LIGHT_GREEN));
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT - 1; i++) 
        putchar(i % 2 == 1 ? ' ' : '#');
    asm("cli");
    for(;;) asm("hlt");
}

void init() {
    terminal_initialize();
    initCharTable();
    kprintf("Init\n");
    init_gdt();
    init_idt();
    init_pic();
    //init_timer(100);
    init_paging(0x100000);
}

void kmain(multiboot_info_t *mbinfo) {
    multiboot_info = mbinfo;
    init();
    kprintf("QSD %x %x %x\n", mbinfo->flags, mbinfo->mods_count, mbinfo->mods_addr);
    asm("sti");
    for(;;) asm("hlt");
    //shell();
    dieSlowly();
}
