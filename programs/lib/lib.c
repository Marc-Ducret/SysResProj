#include "lib.h"

void set_char_at(u8 c, u8 fg, u8 bg, u8 x, u8 y) {
    u16* screen = (u16*) 0x88000000;
    screen[x + y * VGA_WIDTH] = (bg << 0xC) + (fg << 0x8) + c;
}

void clear_screen(u8 color) {
    for(u8 y = 0; y < VGA_HEIGHT; y++)
        for(u8 x = 0; x < VGA_WIDTH; x++)
            set_char_at(' ', color, color, x , y);
}

int next_key_event() {
    void *keyBuff = (void *) 0x88001000;
    u16 *writeCursor = (u16 *) keyBuff;
    u16 *readCursor = (u16 *) keyBuff + 1;
    u8 *eventBuffer = (u8 *) keyBuff + 4;
    if(*readCursor == *writeCursor) return -1;
    else {
        int event = eventBuffer[*readCursor];
        *readCursor = (*readCursor + 1) % KEY_BUFFER_SIZE;
        return event;
    }
}
