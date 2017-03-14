#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "printing.h"
#include "io.h"

void kmain() {
	terminal_initialize();
	unsigned char key = 0;
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


