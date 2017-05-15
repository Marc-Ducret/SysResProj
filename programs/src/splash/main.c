#include "lib.h"

#define SIZE 25

u8 cacatoes[SIZE][SIZE] = 
    {{ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 14,  7,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 15, 15,  0,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7, 14, 14,  7,  0,  0 },
    {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  7, 15, 14, 14,  7,  0,  0 },
    {  0,  0,  0,  0,  0,  7, 15, 15, 15, 15, 15, 15, 15, 15,  7,  8, 15, 15, 14, 14, 14, 14,  7,  0,  0 },
    {  0,  0,  0,  8, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 15,  0,  0 },
    {  0,  0,  0, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  7,  0,  0 },
    {  0,  0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 15,  0,  0,  0 },
    {  0,  8, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 15,  0,  0,  0,  0 },
    {  0,  8, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14,  7,  0,  0,  0,  0,  0 },
    {  0,  8,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15,  8,  0,  0,  0,  0,  0,  0 },
    {  0,  8,  7,  0,  8,  7,  7,  7,  7,  7,  8, 15, 15,  7, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0,  0 },
    {  0,  7,  0,  0,  0,  0,  7,  7,  7,  8,  0,  8, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0 },
    {  0, 15,  7,  8,  8,  0,  8,  7,  7,  7,  8,  7, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0 },
    {  0,  7, 15,  8,  8,  8,  8,  8,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0,  0 },
    {  0,  8,  7,  8,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  7,  0,  0,  0,  0,  0 },
    {  0,  8,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0 },
    {  0,  0,  7,  8, 15, 15,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0 },
    {  0,  0,  7,  8,  7, 15,  7,  7, 15,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0,  0 },
    {  0,  0,  7,  8,  7,  7,  7, 15,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0 },
    {  0,  0,  0,  8,  7,  7,  7, 15,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0 },
    {  0,  0,  0,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15,  0,  0,  0,  0 }};
    
u8 opacity[SIZE*SIZE];

void draw() {
    set_char_at(' ', 0, 0, 0, 0);
    for(u8 y = 0; y < SIZE; y ++) {
        for(u8 x = 0; x < SIZE; x ++) {
            if (cacatoes[y][x]) {
                u8 fg = opacity[x + y * SIZE] ? cacatoes[y][x] : 0;
                u8 bg = opacity[x + y * SIZE] == 4 ? cacatoes[y][x] : 0;
                set_char_at(175 + opacity[x + y * SIZE], fg, bg, 15+2*x  , y);
                set_char_at(175 + opacity[x + y * SIZE], fg, bg, 15+2*x+1, y);
            }
        }
    }
}
u32 norm = 1 << 15;
u32 nb_remaining;
u32 tot;

u32 find(u32 x, u32 a, u32 b) {
    if ((u32) (b - a) <= 1)
        return b;
    u32 c = (a + b) / 2;
    if (c * c <= x) {
        return find(x, c, b);
    }
    else {
        return find(x, a, c);
    }
}

u32 sqrt(u32 x) {
    u32 res = 0;
    //while(res * res <= x) {
    //    res++;
    //}
    res = find(x, 0, (u32) 1 << 15);
    if (res == 0)
        return 0;
    u32 d1 = res * res - x;
    res--;
    u32 d2 = x - res * res;
    return d1 < d2 ? res + 1 : res; 
}

u32 get_ord(int abs) {
    return sqrt((u32) norm * norm - (u32) (abs * abs));
}

void plot(u32 fraction, u32 radius_x, u32 radius_y, u32 abs, u32 ord, int rev, u8 c) {
    int x = (fraction * 2 * radius_x) / norm + (abs - radius_x);
    int y = (get_ord(fraction * 2 - norm) *  radius_y) / norm * (rev ? -1 : 1) + ord;
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT)
        return;
    set_char_at(' ', c, c, x, y);
}

u8 tick() {
    u8 keep_running = 0;
    for(u8 y = 0; y < SIZE; y ++) {
        for(u8 x = 0; x < SIZE; x ++) {
            if(opacity[x + y * SIZE] < 4 && cacatoes[y][x]) {
                keep_running = 1;
                if(!(rand() % min(1 + nb_remaining / (3 * 25), 100000))) {
                    opacity[x + y * SIZE] ++;
                    nb_remaining--;
                    plot(((tot - nb_remaining) * norm) / tot, 40, 25, 40, 25, 1, RED);
                    plot(((tot - nb_remaining) * norm) / tot, 39, 24, 40, 25, 1, LIGHT_BROWN);
                    plot(((tot - nb_remaining) * norm) / tot, 38, 23, 40, 25, 1, GREEN);
                    plot(((tot - nb_remaining) * norm) / tot, 37, 22, 40, 25, 1, CYAN);
                    plot(((tot - nb_remaining) * norm) / tot, 36, 21, 40, 25, 1, BLUE);
                }
            }
        }
    }
    return keep_running;
}

int main(char *args) {
    memset(opacity, 0, SIZE*SIZE);
    tot = 0;
    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            if (cacatoes[y][x])
                tot += 4;
        }
    }
    u8 run = 1;
    int res = -1;
    errno = ENOFOCUS;
    nb_remaining = tot;
    while (res == -1 && errno == ENOFOCUS) {
        res = get_key_event();
    }
    int speed_up = 0;
    set_char_at(' ', BLACK, BLACK, 0, 0);
    while(run) {
        run = tick();
        draw();
        while (get_key_event() != -1)
            speed_up+= 3;
        
        if (speed_up > 0)
            speed_up--;
        else
            sleep(35);
    }
    char *name = "CacatOS";
    char *message = "Press (nearly) any Key to continue";
    int length = strlen(name);
    int x = 38 - length/2;
    int y = 21;

    if (speed_up > 0)
        speed_up--;
    else
        sleep(50);
    for (int j = 0; j < length; j++) {
        u8 fg = BLACK;
        u8 bg = cacatoes[y][(x+j-15)/2];
        set_char_at(name[j], fg, bg, x+j, y);
    }
    
    length = strlen(message);
    x = 39 - length / 2;
    y = 23;
    sleep(50);
    for (int j = 0; j < length; j++) {
        u8 fg = BLACK;
        u8 bg = cacatoes[y][(x+j - 15) / 2];
        set_char_at(message[j], fg, bg, x+j, y);
    }
    u8 available_colors[9] = {0, 1, 2, 4, 5, 6, 8, 12, 13};
    
    while (!speed_up && get_key_event() == -1) {
        sleep(50);
        int j = rand() % length;
        u8 fg = available_colors[rand()%9];
        u8 bg = cacatoes[y][(x+j - 15) / 2];
        set_char_at(message[j], fg, bg, x+j, y);
    }
    int r = 60;
    u32 tot = 500;
    while (r > 0) {
        r--;
        while (get_key_event() != -1)
            speed_up+= 3;
        
        if (speed_up > 0)
            speed_up--;
        else
            sleep(13);
        for (int j = 0; j <= tot; j++) {
            plot((j * norm) / tot, r, r/2, 40, 12, 0, BLACK);
            plot((j * norm) / tot, r, r/2, 40, 12, 1, BLACK);
        }
    }
    exec("/bin/console.bin", "/bin/shell.bin", -1, -1);
    exit(0);
}
