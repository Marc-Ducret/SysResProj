#include "lib.h"
#include "parsing.h"

#define SCROLL_HEIGHT 0x100
#define SCREEN_SIZE (VGA_WIDTH * VGA_HEIGHT * 2)

u8 cursor_x;
u8 cursor_y;
u8 bg_color;
u8 fg_color;
u8 old_bg_color;
u8 old_fg_color;
int scroll_off;
int scroll;
int max_scroll;

u16 scroll_buffer[VGA_WIDTH * SCROLL_HEIGHT];

void c_put_char(u8 c);

void init() {
    clear_screen(WHITE);
    cursor_x = cursor_y = scroll_off = scroll = max_scroll = 0;
    fg_color = old_fg_color = GREEN;
    bg_color = old_bg_color = WHITE;
    for(int i = 0; i < VGA_WIDTH * SCROLL_HEIGHT; i ++)
        scroll_buffer[i] = (bg_color << 0xC) + (fg_color << 0x8);
}

void update_cursor(void) {
    u8 *screen = (u8*) get_screen();
    *(screen + SCREEN_SIZE) = cursor_x;
    *(screen + SCREEN_SIZE + 1) = cursor_y;    
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
    update_cursor();
}

void c_put_char(u8 c) {
    if(c == '\n') {
        while(++cursor_x < VGA_WIDTH) set_char(' ', cursor_x, cursor_y);
        cursor_x--;
    } else if (c == 0x8) {
        if(cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
        }
        set_char(' ', cursor_x, cursor_y);
        return;
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
    set_char(' ', cursor_x, cursor_y);
    if(scroll > 0) {
        scroll = 0;
        load_from_scroll();
    }
}

void print_string(char *s) {
    while (*s)
        c_put_char(*s++);
}

void scroll_down() {
    if(--scroll < 0) scroll = 0;
    else load_from_scroll();
}

void scroll_up() {
    if(++scroll > max_scroll) scroll = max_scroll;
    else load_from_scroll();
}

void erase() {
    u16 *screen = get_screen();
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screen[i] = (bg_color << 0xC) + (fg_color << 0x8) + ' ';
    }
}

int c_parse_char(u8 c) {
    static int esc_seq = 0;
    static int esc_seen = 0;
    if (esc_seen) {
        if (c == '[') {
            esc_seen = 0;
            esc_seq = 1;
            return 1;
        }
        esc_seen = 0;
    }
    if (c == '\e') {
        esc_seen = 1;
        return 1;
    }
    if (!esc_seq) {
        c_put_char(c);
        update_cursor();
        return 1;
    }
    
    // Here, we are in an escape sequence.
    // Every following character codes an action (following our code).
    if (c == 124) {
        // End of escape sequence.
        esc_seq = 0;
    }
    
    if (c < 25) {
        cursor_y = c;
    }
    else if (c < 105) {
        cursor_x = c - (u8) 25;
    }
    else if (c == 105) {
        if (cursor_x > 0) cursor_x--;
    }
    else if (c == 106) {
        if (cursor_x < VGA_WIDTH - 1) cursor_x++;
    }
    else if (c == 107) {
        if (cursor_y > 0) cursor_y--;
    }
    else if (c == 108) {
        if (cursor_y < VGA_HEIGHT - 1) cursor_y++;
    }
    else if (c == 109) {
        if (cursor_x > 0) cursor_x--;
        else if (cursor_y > 0) {
            cursor_x = VGA_WIDTH - 1;
            cursor_y--;
        }
    }
    else if (c == 110) {
        if (cursor_x < VGA_WIDTH - 1) cursor_x++;
        else if (cursor_y < VGA_HEIGHT - 1) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    else if (c < 128) {
        // Uknown character
    }
    else if (c < 144) {
        old_fg_color = fg_color;
        fg_color = c - (u8) 128;
    }
    else if (c < 160) {
        old_bg_color = bg_color;
        bg_color = c - (u8) 144;
    }
    else if (c == 160) {
        // Gets back to previous foreground color.
        fg_color = old_fg_color;
    }
    else if (c == 161) {
        // Gets back to previous background color.
        bg_color = old_bg_color;
    }
    else if (c == 162) {
        // Exit
        return 0;
    }
    else if (c == 163) {
        // Clears the buffer
        for (int i = 0; i < cursor_y; i++)
            c_put_char('\n');
        cursor_x = 0;
        cursor_y = 0;
    }
    else if (c == 164) {
        scroll_up();
    }
    else if (c == 165) {
        scroll_down();
    }
    else if (c == 166) {
        erase();
    }
    update_cursor();
    return 1;
}

void test_create() {
    int chanid = new_channel();
    print_string("New channel : ");
    c_put_char('0' + chanid);
    c_put_char('\n');
}

void test_send(void) {
    rtc_time_t time;
    rtc_time_t *t = &time;
    gettimeofday(t);
    fprintf(0, "%d:%d:%d, %d/%d/%d", t->hours, t->minutes, t->seconds,
            t->day, t->month, (t->year % 100) + 2000);
    int res = flush(0);
    print_string("Oui, reussi a envoyer : ");
    if (res >= 100) {
        c_put_char('0' + res / 100);
        res = res % 100;
    }
    if (res >= 10) {
        c_put_char('0' + res / 10);
        res = res % 10;
    }
    if (res >= 0) {
        c_put_char('0' + res);
    }
    print_string(" !\n");
}

void test_receive(void) {
    pid_t pid = wait_channel(0, 0);
    print_string("Pid : ");
    c_put_char('0' + pid);
    print_string("\n");
    if (pid < 0)
        return;
    char buffer[154];
    memset(buffer, 0, 154);
    int res = receive(0, (u8*)buffer, 154);
    print_string(strerror(errno));
    if (res < 0) {
        print_string("Rate, pas de recu !\n");
        return;
    }
    print_string("voila le message :");
    print_string(buffer);
}

int test_ls() {
    dirent_t dirent;
    fd_t fd = opendir(".");
    printf("HI");
    if (fd < 0) {
        printf("Error in ls : %s\n", strerror(errno));
        return 1;
    }
    int res = readdir(fd, &dirent);
    while (res == 0) {
        printf("%s  ", dirent.name);
        res = readdir(fd, &dirent);
    }
    if (errno != ENOENT) {
        printf("Error in ls : %s\n", strerror(errno));
    }
    closedir(fd);
    return 0;
}

u8 recv_buff[512];
u8 shift, alt;
int key_buffer[10];
int index_new = 0;

int main(char *args) {
    char *file;
    args = parse_arg(args, &file);
    init();
    int in = new_channel();
    int out = new_channel();
    c_put_char('[');
    print_string(file);
    c_put_char(']');
    c_put_char('\n');
    pid_t pid = exec(file, args, out, in);
    int run = (pid != -1);
    while (run) {
        sleep(50);
        key_buffer[index_new] = get_key_event();
        while (key_buffer[index_new] != -1) {
            index_new++;
            key_buffer[index_new] = get_key_event();
        }
        for (int index = 0; index < index_new; index++) {
            int event = key_buffer[index];
            if      ((event & 0x7F) == KEY_SHIFT ) shift = !(event & 0x80);
            else if ((event & 0x7F) == KEY_RSHIFT ) shift = !(event & 0x80);
            else if ((event & 0x7F) == KEY_ALT_GR) alt   = !(event & 0x80);
            //else if (event == KEY_COLON) test_ls();
            else if(event >= 0 && event < 0x80) {
                /*if(event == KEY_SHIFT) scroll_up();
                else if(event == KEY_CTRL) scroll_down();
                else if(event == KEY_ESCAPE) exec("/console.bin", in, out);
                else if(event == KEY_COLON) test_create();
                else if(event == KEY_HAT) test_send();
                else if(event == KEY_RPAR) test_receive();*/
                if(event == KEY_CTRL) scroll_up(); 
                else {
                    char c = getKeyChar(event, shift, alt);
                    if(c) {
                        if (stream_putchar(c, out) == -1)
                            run = 0;
                    }
                }
            }
        }
        if (index_new) {
            int res = flush(out);
            if (res == -1)
                run = 0;
        }
        index_new = 0;
        int ct;
        while ((ct = receive(in, recv_buff, 512)) > 0) {
            for(int i = 0; i < ct; i ++) {
                run = c_parse_char(recv_buff[i]);
                if(!run)
                    break;
            }
            if (!run)
                break;
            sleep(1);
        }
    }
    // Exits.
    print_string("CONSOLE EXITED.");
    int exit_value = kill(pid) ? EXIT_FAILURE : EXIT_SUCCESS;
    exit(exit_value);
}
