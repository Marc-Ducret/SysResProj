#ifndef PRINTING_H
#define PRINTING_H

#include <stdint.h>
#include <stddef.h>

void terminal_initialize();
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void putchar(char c);
void erase();
void putint(int i);
void kprintf(const char* data, ...);

#endif /* PRINTING_H */

