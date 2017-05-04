#include "lib.h"

u16 *get_screen() {
    return (u16*) 0x88000000;
}

void set_char_at(u8 c, u8 fg, u8 bg, u8 x, u8 y) {
    u16* screen = get_screen();
    screen[(x%VGA_WIDTH) + (y%VGA_HEIGHT) * VGA_WIDTH] = (bg << 0xC) + (fg << 0x8) + c;
}

void clear_screen(u8 color) {
    for(u8 y = 0; y < VGA_HEIGHT; y++)
        for(u8 x = 0; x < VGA_WIDTH; x++)
            set_char_at(' ', color, color, x , y);
}
/*
int next_key_event() {
    int res;
    int id = 40;
    asm volatile("\
                movl %1, %%eax \n \
                int $0x80 \n \
                movl %%eax, %0"
                : "=m" (res)
                : "m" (id)
                : "%ebx", "esi", "edi");
    return res;
}*/

u32 last_rand = 1351968;

u32 rand() {
    last_rand = ((last_rand*884519) % 56311523)*8984541;
    return last_rand;
}
