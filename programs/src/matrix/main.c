#include "lib.h"


u8 map[VGA_WIDTH][VGA_HEIGHT];
int neighbors[4];
int possibles;
#define NB_COLORS 4
int colors2[NB_COLORS] = {BLUE, GREEN, RED, LIGHT_BROWN};
int colors1[NB_COLORS] = {MAGENTA, WHITE, CYAN, LIGHT_GREY};

void draw(int c1, int c2, int prev_x, int prev_y, int x, int y, int next_x, int next_y) {
    char c;
    int dx1 = x - prev_x;
    int dx2 = next_x - x;
    int dy1 = y - prev_y;
    int dy2 = next_y - y;
    
    if (dx1 > 1)
        dx1 = -1;
    if (dx2 > 1)
        dx2 = -1;
    if (dy1 > 1)
        dy1 = -1;
    if (dy2 > 1)
        dy2 = -1;
    if (-dx1 > 1)
        dx1 = 1;
    if (-dx2 > 1)
        dx2 = 1;
    if (-dy1 > 1)
        dy1 = 1;
    if (-dy2 > 1)
        dy2 = 1;
    if ((dx1 == 1 && dy2 == 1) || (dy1 == -1 && dx2 == -1)) {
        c = 191;
    }
    else if ((dx1 == 1 && dy2 == -1) || (dy1 == 1 && dx2 == -1)) {
        c = 217;
    }
    else if ((dx1 == -1 && dy2 == -1) || (dy1 == 1 && dx2 == 1)) {
        c = 192;
    }
    else if ((dx1 == -1 && dy2 == 1) || (dy1 == -1 && dx2 == 1)) {
        c = 218;
    }
    else if (dx1 * dx2 == 1) {
        c = 196;
    }
    else if (dy1 * dy2 == 1) {
        c = 179;
    }
    else {
        c = '\t';
    }
    set_char_at(c, c2, BLACK, x, y);
    set_char_at(7, c1, BLACK, next_x, next_y);
}

void init_map(int walls) {
    clear_screen(BLACK);
    memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
    
    if (walls) {
        for (int i = 0; i < VGA_WIDTH; i++) {
            map[i][0] = 1;
            map[i][VGA_HEIGHT-1] = 1;
            set_char_at(' ', DARK_GREY, DARK_GREY, i, 0);
            set_char_at(' ', DARK_GREY, DARK_GREY, i, VGA_HEIGHT-1);
        }
        for (int i = 0; i < VGA_HEIGHT; i++) {
            map[0][i] = 1;
            map[VGA_WIDTH-1][i] = 1;
            set_char_at(' ', DARK_GREY, DARK_GREY, 0, i);
            set_char_at(' ', DARK_GREY, DARK_GREY, VGA_WIDTH-1, i);
        }
    }
}

int main(char *args) {
    memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
    int x[NB_COLORS], y[NB_COLORS], prev_x[NB_COLORS], prev_y[NB_COLORS], next_x[NB_COLORS], next_y[NB_COLORS], c1, c2, nb_run;
    int run[NB_COLORS] = {0};
    clear_screen(BLACK);
    while (1) {
        memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
        init_map(1);
        for (int i = 0; i < NB_COLORS; i++) {
            run[i] = 1;
            x[i] = rand() % VGA_WIDTH;
            y[i] = rand() % VGA_HEIGHT;
            while (map[x[i]][y[i]]) {
                x[i] = rand() % VGA_WIDTH;
                y[i] = rand() % VGA_HEIGHT;
            }
            map[x[i]][y[i]] = 1;
        }
        nb_run = NB_COLORS;
        while (nb_run) {
            sleep(150);
            for (int k = 0; k < NB_COLORS; k++) {
                if (!run[k])
                    continue;
                c1 = colors1[k];
                c2 = colors2[k];
            
                neighbors[0] = !map[x[k]+1][y[k]];
                neighbors[1] = !map[x[k]-1][y[k]];
                neighbors[2] = !map[x[k]][y[k]+1];
                neighbors[3] = !map[x[k]][y[k]-1];
                
                if (!neighbors[0] && !neighbors[1] && !neighbors[2] && !neighbors[3]) {
                    set_char_at('\t', RED, BLACK, x[k], y[k]);
                    run[k] = 0;
                    nb_run--;
                    break;
                }
                
                int choice = rand() % 4;
                while (!neighbors[choice]) {
                    choice = (choice + 1) % 4;
                }

                next_x[k] = x[k];
                next_y[k] = y[k];
                if (choice == 0)
                    next_x[k] = x[k] + 1;
                if (choice == 1)
                    next_x[k] = x[k] - 1;
                if (choice == 2)
                    next_y[k] = y[k] + 1;
                if (choice == 3)
                    next_y[k] = y[k] - 1;
                draw(c1, c2, prev_x[k], prev_y[k], x[k], y[k], next_x[k], next_y[k]);
                prev_x[k] = x[k];
                prev_y[k] = y[k];
                x[k] = next_x[k];
                y[k] = next_y[k];
                map[x[k]][y[k]] = 1;
                
            }
            
        }
        // Died.
        sleep(500);
    }
}