#include "lib.h"

#define CONQUEST_COST 8
#define ANIM_RATIO 2
#define TURNS 100

u8 player_color[2];
u8 neutral_color;
char map[VGA_WIDTH * VGA_HEIGHT];
char new_map[VGA_WIDTH * VGA_HEIGHT];
u32 turn;

u8 abs(char x) {
    return x < 0 ? -x : x;
}

int sgn(int x) {
    if(x) {
        return x < 0 ? -1 : 1;
    } else return 0;
}

char get_at(u8 x, u8 y) {
    return map[x + y * VGA_WIDTH];
}

char get_new_at(u8 x, u8 y) {
    return new_map[x + y * VGA_WIDTH];
}

void set_at(char v, u8 x, u8 y) {
    map[x + y * VGA_WIDTH] = v;
}

void set_new_at(char v, u8 x, u8 y) {
    new_map[x + y * VGA_WIDTH] = v;
}

void incr_new_at(char v, u8 x, u8 y) {
    set_new_at(get_new_at(x, y) + v, x, y);
}

void init() {
    initCharTable();
    player_color[0] = COLOR_RED;
    player_color[1] = COLOR_BLUE;
    neutral_color = COLOR_BLACK;
    clear_screen(COLOR_WHITE);
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        map[i] = 0;
    }
    turn = 0;
}

void spread(u8 x, u8 y, char sign) {
    if(x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    incr_new_at(sign, x, y);
}

char bound(char x, char min, char max) {
    if(x > max) return max;
    if(x < min) return min;
    return x;
}

void play(u8 player) {
    char sign = 2*player - 1;
    
    u32 pos = rand() % (VGA_WIDTH * VGA_HEIGHT);
    
    incr_new_at(sign * CONQUEST_COST, pos % VGA_WIDTH, pos / VGA_WIDTH);
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
    play(0);
    play(1);
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        map[i] = bound(new_map[i], -CONQUEST_COST, CONQUEST_COST);
    }
    turn++;
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
                if(v/ANIM_RATIO) {
                    fg = player_color[(sgn(v) + 1)/2];
                    c = 175 + abs(v)/ANIM_RATIO;
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
    while(turn < TURNS) {
        int k = next_key_event();
        if(k > 0 && k < 0x80) tick();
        draw();
        asm("hlt");
    }
    int score = 0;
    for(u8 y = 0; y < VGA_HEIGHT; y++) {
        for(u8 x = 0; x < VGA_WIDTH; x++) {
            score += get_at(x, y);
        }
    }
    u8 bg = COLOR_WHITE;
    u8 fg;
    if(score == 0) fg = neutral_color;
    else fg = player_color[(sgn(score)+1)/2];
    for(u8 y = 0; y < VGA_HEIGHT; y++) {
        for(u8 x = 0; x < VGA_WIDTH; x++) {
            set_char_at('#', fg, bg, x, y);
        }
    }
    for(;;) asm("hlt");
}
