#include "lib.h"


u8 map[VGA_HEIGHT][VGA_WIDTH];
int neighbors[4];
int possibles;

void draw(int prev_x, int prev_y, int x, int y, int next_x, int next_y) {
    set_char_at('\t', BLUE, BLACK, x, y);
}

int main(char *args) {
    memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
    int x, y, prev_x, prev_y, next_x, next_y, run;
    clear_screen(BLACK);
    while (1) {
        memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
        clear_screen(BLACK);
        x = rand() % VGA_WIDTH;
        y = rand() % VGA_HEIGHT;
        map[x][y] = 1;
        run = 1;
        while (run) {
            sleep(100);
            possibles = 0;
            for (int i = -1; i <= 1; i += 2) {
                neighbors[(i+1) / 2] = !map[(x+i+VGA_WIDTH)%VGA_WIDTH][y];
                if (neighbors[(i+1) / 2]) possibles++;
                
            }
            for (int j = -1; j <= 1; j+=2) {
                neighbors[(j + 1) /2 + 2] = !map[x][(y+j+VGA_HEIGHT)%VGA_HEIGHT];
                if (neighbors[(j + 1) /2 + 2]) possibles++;
            }
            
            if (!possibles) {
                run = 0;
                break;
            }
            int choice = ((rand() % possibles) + possibles) % possibles;
            int neigh = 0;
            while (choice > 0 || !neighbors[neigh]) {
                if (neighbors[neigh])
                    choice--;
                neigh++;
            }
            int delta = (neigh % 2) ? 1 : -1;
            int is_x = neigh < 2;
            next_x = (x + (is_x ? delta : 0) + VGA_WIDTH) % VGA_WIDTH;
            next_y = (y + (!is_x ? delta : 0) + VGA_HEIGHT) % VGA_HEIGHT;
            
            draw(prev_x, prev_y, x, y, next_x, next_y);
            printf("%d, %d --> %d, %d --> %d, %d\n", prev_x, prev_y, x, y, next_x, next_y);
            prev_x = x;
            prev_y = y;
            x = next_x;
            y = next_y;
            map[x][y] = 1;
        }
        set_char_at('\t', RED, BLACK, x, y);
        // Died.
        sleep(2000);
        clear_screen(BLUE);
        sleep(500);
    }
}