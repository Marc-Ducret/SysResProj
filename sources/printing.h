#ifndef PRINTING_H
#define PRINTING_H

#include "int.h"
#include "lib.h"
#include "stream.h"
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GREY 7
#define COLOR_DARK_GREY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_LIGHT_BROWN 14
#define COLOR_WHITE 15

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define SCROLL_HEIGHT 256
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * 2)

u8 make_color(u8 fg, u8 bg);
void terminal_initialize();
void terminal_setcolor(u8 color);
void terminal_putentryat(char c, u8 color, u32 x, u32 y);
void putchar(char c);
void erase();
void putint(int i);
void gputint(int i, stream_t *stream);
void kprintf(const char* data, ...);
void fprintf(stream_t *stream, const char* data, ...);
void clear(u8 bgColor);
void scrollup();
void scrolldown();
char *write_int(char *buffer, int x);
#endif

