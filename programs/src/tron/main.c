#include "lib.h"
#include "parsing.h"


u8 map[VGA_WIDTH][VGA_HEIGHT];
int neighbors[4];
int possibles;
#define NB_COLORS 4
int colors2[NB_COLORS+1] = {BLUE, GREEN, RED, LIGHT_BROWN, WHITE};
int colors1[NB_COLORS+1] = {MAGENTA, WHITE, CYAN, LIGHT_GREY, GREEN};

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

void pause() {
    int key = 0;
    while (key != KEY_SPACE) {
        sleep(100);
        key = get_key_event();
        if (key == KEY_ESCAPE)
            exit(EXIT_SUCCESS);
    }
}

void check_key(int *direction) {
    int key;
    while (1) {
        errno = ECLEAN;
        key = get_key_event();
        while (key != -1) {
            if (key == KEY_UP) {
                direction[0] = 0;
                direction[1] = -1;
            }
            if (key == KEY_DOWN) {
                direction[0] = 0;
                direction[1] = 1;
            }
            if (key == KEY_LEFT) {
                direction[0] = -1;
                direction[1] = 0;
            }
            if (key == KEY_RIGHT) {
                direction[0] = 1;
                direction[1] = 0;
            }
            if (key == KEY_ESCAPE)
                exit(EXIT_SUCCESS);
            if (key == KEY_SPACE)
                break;
            errno = ECLEAN;
            key = get_key_event();
        }
        if (errno != ENOFOCUS && key != KEY_SPACE)
            return;
        pause();
    }
}

int value(int x, int y) {
    if(x < 0 || y < 0 || x >= VGA_WIDTH || y >= VGA_HEIGHT) return 1;
    return map[x][y];
}

int sq(int x) {
    return x*x;
}

int density(int x, int y, int s) {
    int ct = 0;
    for(int i = x - s; i <= x + s; i++) {
        for(int j = y - s; j <= y + s; j++) {
            ct += value(i, j) * (2*sq(s) - sq(x-i) - sq(y-j));
        }
    }
    return ct * 10;
}

int main(char *args) {
    args_t params;
    int err = parse(args, &params);
    if (err == -1 || params.nb_args)
        too_many_args("tron");
    
    int level = !(eat_option(&params, "r"));
    int is_player = !(eat_option(&params, "nop"));
    remain_option(&params, "tron");
    memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
    int x[NB_COLORS+1], y[NB_COLORS+1], prev_x[NB_COLORS+1], prev_y[NB_COLORS+1];
    int next_x[NB_COLORS+1], next_y[NB_COLORS+1], c1, c2, nb_run;
    int p = NB_COLORS;
    int run[NB_COLORS + 1] = {0};
    int nb = NB_COLORS + 1;
    int direction[2]; // Player direction
    clear_screen(BLACK);
    update_clock(WHITE, BLACK);
    while (get_key_event() == -1 && errno == ENOFOCUS) {
        sleep(10);
    }
    while (1) {
        memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
        init_map(1);
        for (int i = 0; i < nb; i++) {
            run[i] = 1;
            x[i] = rand() % VGA_WIDTH;
            y[i] = rand() % VGA_HEIGHT;
            while (map[x[i]][y[i]]) {
                x[i] = rand() % VGA_WIDTH;
                y[i] = rand() % VGA_HEIGHT;
            }
            set_char_at('\t', colors2[i], BLACK, x[i], y[i]);
            prev_x[i] = x[i];
            prev_y[i] = y[i];
            map[x[i]][y[i]] = 1;
            direction[0] = 1;
            direction[1] = 0;
        }
        nb_run = NB_COLORS + is_player;
        if (is_player)
            pause();
        else {
            set_char_at(' ', BLACK, BLACK, x[p], y[p]);
            map[x[p]][y[p]] = 0;
        }
        while (nb_run) {
            if (is_player)
                sleep(100);
            else 
                sleep(150);
            check_key(direction);
            for (int k = 0; k < NB_COLORS; k++) {
                if (!run[k])
                    continue;
                c1 = colors1[k];
                c2 = colors2[k];
                
                int choice;
                if (level) {
                    int s = 2;

                    neighbors[0] = density(x[k]+1, y[k]  , s);
                    neighbors[1] = density(x[k]-1, y[k]  , s);
                    neighbors[2] = density(x[k]  , y[k]+1, s);
                    neighbors[3] = density(x[k]  , y[k]-1, s);

                    int min = neighbors[0];
                    choice = 0;
                    for(int i = 0; i < 4; i ++) if(neighbors[i] < min || (neighbors[i] == min && rand()%2)) {
                        min = neighbors[i];
                        choice = i;
                    }
                }
                else {
                    neighbors[0] = !map[x[k]+1][y[k]];
                    neighbors[1] = !map[x[k]-1][y[k]];
                    neighbors[2] = !map[x[k]][y[k]+1];
                    neighbors[3] = !map[x[k]][y[k]-1];
                    if (!neighbors[0] && !neighbors[1] && !neighbors[2] && !neighbors[3]) {
                        choice = 0;
                    }
                    else {
                        choice = rand() % 4;
                        while (!neighbors[choice]) {
                            choice = (choice + 1) % 4;
                        }
                    }
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
                if(map[next_x[k]][next_y[k]]) {
                    set_char_at('\t', RED, BLACK, x[k], y[k]);
                    run[k] = 0;
                    nb_run--;
                    continue;
                }
                draw(c1, c2, prev_x[k], prev_y[k], x[k], y[k], next_x[k], next_y[k]);
                prev_x[k] = x[k];
                prev_y[k] = y[k];
                x[k] = next_x[k];
                y[k] = next_y[k];
                map[x[k]][y[k]] = 1;
                
            }
            if (run[p] && is_player) {
                check_key(direction);
                // For the player
                next_x[p] = x[p] + direction[0];
                next_y[p] = y[p] + direction[1];
                if (map[next_x[p]][next_y[p]]) {
                    run[p] = 0;
                    nb_run--;
                    set_char_at('\t', RED, BLACK, x[p], y[p]);
                    break;
                }
                draw(colors1[p], colors2[p], prev_x[p], prev_y[p], x[p], y[p], next_x[p], next_y[p]);
                prev_x[p] = x[p];
                prev_y[p] = y[p];
                x[p] = next_x[p];
                y[p] = next_y[p];
                
                
                
                map[x[p]][y[p]] = 1;
            }
        }
        // Died.
        if (is_player)
            pause();
        else
            sleep(700);
    }
}
