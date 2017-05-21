#include "lib.h"
#include "parsing.h"


u8 map[VGA_WIDTH][VGA_HEIGHT];
int neighbors[4];
int possibles;
#define NB_COLORS 4
int colors2[NB_COLORS+2] = {BLUE, GREEN, RED, LIGHT_BROWN, WHITE, MAGENTA};
int colors1[NB_COLORS+2] = {MAGENTA, WHITE, CYAN, LIGHT_GREY, GREEN, BROWN};

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
        update_clock(WHITE, BLACK);
        sleep(100);
        key = get_key_event();
        if (key == KEY_ESCAPE)
            exit(EXIT_SUCCESS);
    }
}

void check_key(int direction[2][2]) {
    int key;
    while (1) {
        errno = ECLEAN;
        key = get_key_event();
        while (key != -1) {
            if (key == KEY_UP) {
                direction[0][0] = 0;
                direction[0][1] = -1;
            }
            if (key == KEY_DOWN) {
                direction[0][0] = 0;
                direction[0][1] = 1;
            }
            if (key == KEY_LEFT) {
                direction[0][0] = -1;
                direction[0][1] = 0;
            }
            if (key == KEY_RIGHT) {
                direction[0][0] = 1;
                direction[0][1] = 0;
            }
            if (key == KEY_Z) {
                direction[1][0] = 0;
                direction[1][1] = -1;
            }
            if (key == KEY_S) {
                direction[1][0] = 0;
                direction[1][1] = 1;
            }
            if (key == KEY_Q) {
                direction[1][0] = -1;
                direction[1][1] = 0;
            }
            if (key == KEY_D) {
                direction[1][0] = 1;
                direction[1][1] = 0;
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
    int two_p = eat_option(&params, "pvp");
    remain_option(&params, "tron");
    if (two_p) 
        is_player = 2;
    
    memset(map, 0, VGA_HEIGHT * VGA_WIDTH * 4);
    int x[NB_COLORS+2], y[NB_COLORS+2], prev_x[NB_COLORS+2], prev_y[NB_COLORS+2];
    int next_x[NB_COLORS+2], next_y[NB_COLORS+2], c1, c2, nb_run;
    int p = NB_COLORS;
    int run[NB_COLORS + 2] = {0};
    int nb = NB_COLORS + is_player;
    int direction[2][2]; // Player direction
    int winner = -1;
    clear_screen(BLACK);
    switch_clock();
    update_clock(WHITE, BLACK);
    while (get_key_event() == -1 && errno == ENOFOCUS) {
        sleep(50);
        update_clock(WHITE, BLACK);
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
            
        }
        direction[0][0] = 1;
        direction[0][1] = 0;
        direction[1][0] = 1;
        direction[1][1] = 0;
        nb_run = NB_COLORS + is_player;
        if (is_player)
            pause();
        while (nb_run) {
            update_clock(WHITE, BLACK);
            if (is_player)
                sleep(100);
            else 
                sleep(150);
            check_key(direction);
            winner = -1;

            for (int k = 0; k < NB_COLORS; k++) {
                if (!run[k])
                    continue;
                if (nb_run == 1) {
                    nb_run = 0;
                    winner = k;
                    break;
                }
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
            for (int k = 0; k < is_player; k++) {
                if (run[p+k]) {
                    if (nb_run == 1) {
                        nb_run = 0;
                        winner = p + k;
                        break;
                    }
                    check_key(direction);
                    // For the player
                    next_x[p+k] = x[p+k] + direction[k][0];
                    next_y[p+k] = y[p+k] + direction[k][1];
                    if (map[next_x[p+k]][next_y[p+k]]) {
                        run[p+k] = 0;
                        nb_run--;
                        set_char_at('\t', RED, BLACK, x[p+k], y[p+k]);
                        break;
                    }
                    draw(colors1[p+k], colors2[p+k], prev_x[p+k], prev_y[p+k], x[p+k], y[p+k], next_x[p+k], next_y[p+k]);
                    prev_x[p+k] = x[p+k];
                    prev_y[p+k] = y[p+k];
                    x[p+k] = next_x[p+k];
                    y[p+k] = next_y[p+k];



                    map[x[p+k]][y[p+k]] = 1;
                }
            }
            
        }
        // Died.
        if (is_player && winner != -1) {
            sleep(1000);
            clear_screen(colors2[winner]);
            sleep(1000);
        }
        else
            sleep(700);
    }
}
