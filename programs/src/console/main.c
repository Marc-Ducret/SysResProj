#include "lib.h"
#include "parsing.h"

#define SCROLL_HEIGHT 0x200

u8 cursor_x;
u8 cursor_y;
u8 bg_color;
u8 fg_color;
u8 old_bg_color;
u8 old_fg_color;
int scroll_off;
int scroll;
int max_scroll;
int clock_mode = 1;
u16 scroll_buffer[VGA_WIDTH * SCROLL_HEIGHT];

void c_put_char(u8 c);

void init() {
    clear_screen(BLACK);
    cursor_x = cursor_y = scroll_off = scroll = max_scroll = 0;
    fg_color = old_fg_color = WHITE;
    bg_color = old_bg_color = BLACK;
    for(int i = 0; i < VGA_WIDTH * SCROLL_HEIGHT; i ++)
        scroll_buffer[i] = (bg_color << 0xC) + (fg_color << 0x8);
}

void update_cursor(void) {
    set_cursor(scroll ? -1 : cursor_x, scroll ? -1 : cursor_y);    
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
    else if (c == 167) {
        switch_clock();
    }
    update_cursor();
    return 1;
}

u8 recv_buff[512];
u8 shift, alt, ctrl;
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
    switch_clock();
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
            else if ((event & 0x7F) == KEY_CTRL) ctrl = !(event & 0x80);
            else if(event >= 0 && event < 0x80) {
                if (event == KEY_D && ctrl) stream_putchar(TERMINATION_KEY, out);
                else if(event == KEY_LEFT) scroll_up();
                else if (event == KEY_RIGHT) scroll_down();
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
        if (clock_mode)
            update_clock(fg_color, bg_color);
    }
    // Exits.
    print_string("CONSOLE EXITED.");
    int exit_value = kill(pid) ? EXIT_FAILURE : EXIT_SUCCESS;
    exit(exit_value);
}
