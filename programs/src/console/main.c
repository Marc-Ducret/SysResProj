#include "lib.h"

#define SCROLL_HEIGHT 0x100

u8 cursor_x;
u8 cursor_y;
u8 bg_color;
u8 fg_color;
int scroll_off;
int scroll;
int max_scroll;

u16 scroll_buffer[VGA_WIDTH * SCROLL_HEIGHT];

void c_put_char(u8 c);

void init() {
    clear_screen(COLOR_WHITE);
    cursor_x = cursor_y = scroll_off = scroll = max_scroll = 0;
    fg_color = COLOR_GREEN;
    bg_color = COLOR_WHITE;
    for(int i = 0; i < VGA_WIDTH * SCROLL_HEIGHT; i ++)
        scroll_buffer[i] = (bg_color << 0xC) + (fg_color << 0x8);
    initCharTable();
}

int index(u8 x, u8 y) {
    return (x % VGA_WIDTH) + (y % VGA_HEIGHT) * VGA_WIDTH;
}

void set_char(u8 c, u8 x, u8 y) {
    set_char_at(c, fg_color, bg_color, x, y);
    scroll_buffer[x + ((y+scroll_off) % SCROLL_HEIGHT)*VGA_WIDTH] = (bg_color << 0xC) + (fg_color << 0x8) + c;
}


void load_from_scroll() {
    u16* screen = get_screen();
    for(int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i ++) {
        int y = (i/VGA_WIDTH + scroll_off - scroll) % SCROLL_HEIGHT;
        screen[i] = scroll_buffer[(i%VGA_WIDTH) + y * VGA_WIDTH];
    }
    //updatecursor(); TODO
}

void c_put_char(u8 c) {
    if(c == '\n') {
        while(++cursor_x < VGA_WIDTH) set_char(' ', cursor_x, cursor_y);
        cursor_x--;
    } else set_char(c, cursor_x, cursor_y);
    if(++cursor_x == VGA_WIDTH) {
        cursor_x = 0;
        if(++cursor_y == VGA_HEIGHT) {
            scroll_off++;
            u16 *screen = get_screen();
            if(++max_scroll > SCROLL_HEIGHT-VGA_HEIGHT) max_scroll = SCROLL_HEIGHT-VGA_HEIGHT;
            for(u8 y = 0; y < VGA_HEIGHT-1; y++) 
                for(u8 x = 0; x < VGA_WIDTH; x++)
                    screen[index(x, y)] = screen[index(x, y+1)];
            cursor_y--;
            for(u8 x = 0; x < VGA_WIDTH; x++) set_char(' ', x, cursor_y);
        }
    }
    if(scroll > 0) {
        scroll = 0;
        load_from_scroll();
    }
}

void scroll_down() {
    if(--scroll < 0) scroll = 0;
    else load_from_scroll();
}

void scroll_up() {
    if(++scroll > max_scroll) scroll = max_scroll;
    else load_from_scroll();
}


int main() {
    init();
    for(;;) {
        int event = kget_key_event();
        if(event >= 0 && event < 0x80) {
            if(event == KEY_SHIFT) scroll_up();
            else if(event == KEY_CTRL) scroll_down();
            else {
                char c = getKeyChar(event);
                if(c) c_put_char(c);
            }
        }
    }
}
