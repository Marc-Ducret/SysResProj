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
            u8 fg = opacity[x + y * SIZE] ? cacatoes[y][x] : 0;
            u8 bg = opacity[x + y * SIZE] == 4 ? cacatoes[y][x] : 0;
            set_char_at(175 + opacity[x + y * SIZE], fg, bg, 15+2*x  , y);
            set_char_at(175 + opacity[x + y * SIZE], fg, bg, 15+2*x+1, y);
        }
    }
}

u8 tick() {
    u8 keep_running = 0;
    u32 static nb_remaining = 4 * SIZE * SIZE;
    for(u8 y = 0; y < SIZE; y ++) {
        for(u8 x = 0; x < SIZE; x ++) {
            if(opacity[x + y * SIZE] < 4 && cacatoes[y][x]) {
                keep_running = 1;
                if(!(rand() % min(nb_remaining/4/25, 10))) {
                    opacity[x + y * SIZE] ++;
                    nb_remaining++;
                }
            }
        }
    }
    return keep_running;
}
    
int main(char *args) {
    memset(opacity, 0, SIZE*SIZE);
    u8 run = 1;
    int res = -1;
    errno = ENOFOCUS;
    while (res == -1 && errno == ENOFOCUS) {
        res = get_key_event();
    }
    while(run) {
        run = tick();
        draw();
        sleep(30);
    }
    exec("/bin/console.bin", "/bin/shell.bin", -1, -1);
    exit(0);
}
