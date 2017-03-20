#include "isr.h"
#include "printing.h"
#include "io.h"
#include "keyboard.h"

void test() {
	putchar('.');
}

void isr_handler(int id) {
	if(id == 1) {
		while ((inportb(0x64) & 0x01) == 0);
		provideKeyEvent(inportb(0x60));
	}
	if(id == 0x3) kprintf("\n####\n");
	outportb(0x20,0x20);
	outportb(0xA0,0x20);
}

