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
    init_error_msg();
    clear_screen(COLOR_WHITE);
    cursor_x = cursor_y = scroll_off = scroll = max_scroll = 0;
    fg_color = COLOR_GREEN;
    bg_color = COLOR_WHITE;
    for(int i = 0; i < VGA_WIDTH * SCROLL_HEIGHT; i ++)
        scroll_buffer[i] = (bg_color << 0xC) + (fg_color << 0x8);
    initCharTable();
    c_put_char('>');
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

void test_create() {
    int chanid = new_channel();
    print_string("New channel : ");
    c_put_char('0' + chanid);
    c_put_char('\n');
}

void test_send(void) {
    fprintf(0, "Bonjour, j'envoie ca dans le channel %d\n", 0);
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
    pid_t pid = wait_channel(0);
    print_string("Pid : ");
    c_put_char('0' + pid);
    print_string("\n");
    if (pid < 0)
        return;
    char buffer[154];
    memset(buffer, 0, 154);
    int res = receive(0, buffer, 154);
    print_string(strerror(errno));
    if (res < 0) {
        print_string("Rate, pas de recu !\n");
        return;
    }
    print_string("voila le message :");
    print_string(buffer);
}

int main(char *args) {
    init();
    int in = new_channel();
    int out = new_channel();
    char *s = args;
    while(*s) c_put_char(*(s++));
    c_put_char('\n');
    exec("/p.bin", out, in);
    for(;;) {
        int event = get_key_event();
        if(event >= 0 && event < 0x80) {
            if(event == KEY_SHIFT) scroll_up();
            else if(event == KEY_CTRL) scroll_down();
            else if(event == KEY_ESCAPE) exec("/console.bin", in, out);
            else if(event == KEY_COLON) test_create();
            else if(event == KEY_HAT) test_send();
            else if(event == KEY_RPAR) test_receive();
            else {
                char c = getKeyChar(event);
                if(c) c_put_char(c);
            }
        }
        u8 c;
        if(receive(in, &c, 1) > 0) {
            c_put_char(c);
        }
    }
}
