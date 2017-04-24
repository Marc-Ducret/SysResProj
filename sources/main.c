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
#include "disk.h"
#include "filesystem.h"
#include "partition.h"
#include "fs_call.h"
#include "stream.h"

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
    init_timer(10000);
    init_paging(0x100000);
    context_t ctx;
    picoinit(&ctx);
    init_disk(0);
    init_fs(0); // init_fs(1) to obtain much more details on the file system !
    init_root();
    init_stderr(NULL);
    test_dir();
    asm("sti");
}

void kmain() {
    init();
    shell();
    dieSlowly();
}
