#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "picokernel.h"
#include "printing.h"

unsigned char inportb (unsigned short _port) {
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}
void outportb (unsigned short _port, unsigned char _data) {
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

void kmain() {
	terminal_initialize();
	putint(1526);
	unsigned char key;
        launch();
	while(true) {
		unsigned char cur = inportb(0x60);
		if(cur != key) {
			key = cur;
			if(key >= 128) {
				putchar('R');
				putint(key - 128);
			} else {
				putchar('P');
				putint(key);
			}
			putchar(' ');
		}
	}
}


