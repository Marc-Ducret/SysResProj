#ifndef LIB_H
#define LIB_H
#include "int.h"
#include "keycode.h"

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHT_GREY 7
#define DARK_GREY 8
#define LIGHT_BLUE 9
#define LIGHT_GREEN 10
#define LIGHT_CYAN 11
#define LIGHT_RED 12
#define LIGHT_MAGENTA 13
#define LIGHT_BROWN 14
#define WHITE 15

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define NULL (void *) 0
#define SCREEN 0x88000000
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * 2)
#define CURSOR_X (SCREEN + SCREEN_SIZE)
#define CURSOR_Y (SCREEN + SCREEN_SIZE + 1)
#define CLOCK (SCREEN + SCREEN_SIZE + 2)

u16 *get_screen();
void set_char_at(u8 c, u8 fg, u8 bg, u8 x, u8 y);
void clear_screen(u8 color);
void switch_clock();
void update_clock(u8 fg, u8 bg);
void set_cursor(u8 x, u8 y);
u32 rand();
void *memcpy(void *dst, void *src, u32 n);
void memset(void *dst, u8 src, u32 len);
int strEqual(char *strA, char *strB);
void strCopy(char *src, char* dest);
u32 strlen(const char* str);
int min(int a, int b);
int max(int a, int b);
u32 umax(u32 a, u32 b);
u32 umin(u32 a, u32 b);

#include "malloc.h"
#include "syscall.h"
#include "args.h"
#include "printing.h"
#include "error.h"
#endif
