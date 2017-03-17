#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "printing.h"
#include "io.h"
#include "gdt.h"
#include "keycode.h"
#include "shell.h"

void kmain() {
	terminal_initialize();
	initCharTable();
    kprintf("Init\n");
    init_gdt();
    shell();
    dieSlowly();
}

void dieSlowly() {
    for(;;);
}


