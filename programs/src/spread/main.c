#include "lib.h"

#define CONQUEST_COST 4

u8 player_color[2];
u8 neutral_color;
char map[VGA_WIDTH * VGA_HEIGHT];
char new_map[VGA_WIDTH * VGA_HEIGHT];

u8 abs(char x) {
    return x < 0 ? -x : x;
}

char sgn(char x) {
    if(x) {
        return x < 0 ? -1 : 1;
    } else return 0;
}

char get_at(u8 x, u8 y) {
    return map[x + y * VGA_WIDTH];
}

void set_at(char v, u8 x, u8 y) {
    map[x + y * VGA_WIDTH] = v;
}

void set_new_at(char v, u8 x, u8 y) {
    new_map[x + y * VGA_WIDTH] = v;
}

void init() {
    player_color[0] = COLOR_RED;
    player_color[1] = COLOR_BLUE;
    neutral_color = COLOR_BLACK;
    clear_screen(COLOR_WHITE);
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        map[i] = 0;
    }
    set_at(+CONQUEST_COST, 2, 2);
    set_at(-CONQUEST_COST, VGA_WIDTH-1-2, VGA_HEIGHT-1-2);
}

void spread(u8 x, u8 y, char sign) {
    if(x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    char v = get_at(x, y);
    if(abs(v) < CONQUEST_COST || sign != sgn(v)) set_new_at(v + sign, x, y);
}

void tick() {
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        new_map[i] = map[i];
    }
    for(u8 y = 0; y < VGA_HEIGHT; y++) {
        for(u8 x = 0; x < VGA_WIDTH; x++) {
            char v = get_at(x, y);
            if(abs(v) == CONQUEST_COST) {
                char sign = sgn(v);
                spread(x+1, y  , sign);
                spread(x-1, y  , sign);
                spread(x  , y+1, sign);
                spread(x  , y-1, sign);
            }
        }
    }
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        map[i] = new_map[i];
    }
}

void draw() {
    for(u8 y = 0; y < VGA_HEIGHT; y++) {
        for(u8 x = 0; x < VGA_WIDTH; x++) {
            char v = get_at(x, y);
            u8 bg, fg;
            char c;
            if(abs(v) == CONQUEST_COST) {
                bg = player_color[(v + CONQUEST_COST) / (2 * CONQUEST_COST)];
                fg = bg;
                c = ' ';
            } else {
                bg = neutral_color;
                if(v) {
                    fg = player_color[(sgn(v) + 1)/2];
                    c = 175 + abs(v);
                } else {
                    fg = bg;
                    c = ' ';
                }
            }
            set_char_at(c, fg, bg, x, y);
        }
    }
}

int main() {
    init();
    for(;;) {
        int k = next_key_event();
        if(k > 0 && k < 0x80) tick();
        draw();
        asm("hlt");
    }
}
