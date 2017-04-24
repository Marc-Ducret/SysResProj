#include "args.h"
#include "io.h"
#include "printing.h"


u8 make_color(u8 fg, u8 bg) {
    return fg | bg << 4;
}

u16 make_vgaentry(char c, u8 color) {
    u16 c16 = c;
    u16 color16 = color;
    return c16 | color16 << 8;
}

int terminal_row;
int terminal_column;
u8 terminal_color;
u16* terminal_buffer;
u16 scroll_buffer[VGA_WIDTH * SCROLL_HEIGHT];
int scroll_off;
int scroll;
int max_scroll;

void terminal_initialize() {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = make_color(COLOR_LIGHT_BLUE, COLOR_BLACK);
    terminal_buffer = (u16*) 0xB8000;
    for (u32 y = 0; y < VGA_HEIGHT; y++)
        for (u32 x = 0; x < VGA_WIDTH; x++) {
            const u32 index = y * VGA_WIDTH + x;
            terminal_buffer[index] = make_vgaentry(' ', terminal_color);
        }
}

void terminal_setcolor(u8 color) {
    terminal_color = color;
}

u32 index(u32 x, u32 y) {
    return x + y * VGA_WIDTH;
}


void terminal_putentryat(char c, u8 color, u32 x, u32 y) {
    terminal_buffer[index(x, y)] = make_vgaentry(c, color);
    scroll_buffer[index(x, (y+scroll_off) % SCROLL_HEIGHT)] = make_vgaentry(c, color);
}

void updatecursor() {
    unsigned short pos = terminal_column + VGA_WIDTH * terminal_row;
    if(scroll > 0) pos = -1;
    outportb(0x3D4, 0x0F);
    outportb(0x3D5, (unsigned char) (pos & 0xFF));
    outportb(0x3D4, 0x0E);
    outportb(0x3D5, (unsigned char) ((pos >> 8) & 0xFF));
}

void load_from_scroll() {
    for(int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i ++) {
        int y = (i/VGA_WIDTH + scroll_off - scroll) % SCROLL_HEIGHT;
        terminal_buffer[i] = scroll_buffer[i%VGA_WIDTH + y * VGA_WIDTH];
    }
    updatecursor();
}

void erase() {
    terminal_column--;
    if(terminal_column < 0) {
        terminal_row--;
        terminal_column = VGA_WIDTH-1;
        if(terminal_row < 0) terminal_row = 0;
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
    updatecursor();
}

void clear(u8 color) {
    for(int x = 0; x < VGA_WIDTH; x ++)
        for(int y = 0; y < VGA_HEIGHT; y ++)
            terminal_putentryat(' ', color, x, y);
    terminal_row = terminal_column = 0;
    updatecursor();
    max_scroll = 0;
    scroll = 0;
}

void scrolldown() {
    if(--scroll < 0) scroll = 0;
    else load_from_scroll();
}

void scrollup() {
    if(++scroll > max_scroll) scroll = max_scroll;
    else load_from_scroll();
}

void putchar(char c) {
    if (c == '\n') {
        while (terminal_column < VGA_WIDTH) {
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
            terminal_column ++;
        }
        terminal_column --;
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    }
    if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            scroll_off ++;
            if(++max_scroll > SCROLL_HEIGHT-VGA_HEIGHT) max_scroll = SCROLL_HEIGHT-VGA_HEIGHT;
            for(u32 row = 0; row < VGA_HEIGHT-1; row ++) 
                for(u32 col = 0; col < VGA_WIDTH; col++)
                    terminal_buffer[index(col, row)] = terminal_buffer[index(col, row+1)];
            for(u32 col = 0; col < VGA_WIDTH; col++)
                terminal_putentryat(' ', terminal_color, col, VGA_HEIGHT-1);
            terminal_row = VGA_HEIGHT-1;
        }
    }
    updatecursor();
    if(scroll > 0) {
        scroll = 0;
        load_from_scroll();
    }    
}

int gputchar(char c, stream_t *stream) {
    int res = 0;
    if (stream == NULL)
        putchar(c);
    else
        res = stream_putchar(c, stream);
    return res;
}

void gputint(int i, stream_t *stream) {
    if(i < 0) {
        gputchar('-', stream);
        gputint(-i, stream);
    } else {
        if(i >= 10) gputint(i / 10, stream);
        gputchar('0' + (i % 10), stream);
    }
}

void putint(int i) {
    gputint(i, NULL);
}

void gputint_hex(u32 i, stream_t *stream) {
    if(i >= 16) gputint_hex(i >> 4, stream);
    u32 number = i % 16;
    gputchar((number < 10) ? ('0' + number) : ('A' + number - 10), stream);
}

void putint_hex(u32 i) {
    gputint(i, NULL);
}

void gprint_string(char *s, stream_t *stream) {
    u32 datalen = strlen(s);
    for ( u32 i = 0; i < datalen; i++ )
        gputchar(s[i], stream);
}

void print_string(char *s) {
    gprint_string(s, NULL);
}

void vkprintf(stream_t *stream, const char* data, va_list args) {    
    char c = *data;
    while (c != 0) {
        if (c == '%') {
            data ++;
            c = *data;
            int nb;
            char *s;
            
            switch (c) {
                case 'd':
                    nb = va_arg(args, int);
                    gputint(nb, stream);
                    break;
                
                case 's':
                    s = va_arg(args, char*);
                    gprint_string(s, stream);
                    break;
                
                case 'x':
                    gprint_string("0x", stream);
                
                case 'h':
                    nb = va_arg(args, unsigned int);
                    gputint_hex(nb, stream);
                    break;
                
                default:
                    break;
            }
        } else {
            gputchar(c, stream);
        }
        
        data ++;
        c = *data;
    }    
    va_end(args);
}

void kprintf(const char* data, ...) {
    va_list args;
    va_start(args, data);
    vkprintf(NULL, data, args);
    va_end(args);
}

void fprintf(stream_t *stream, const char *data, ...) {
    //putchar('x');
    va_list args;
    va_start(args, data);
    //vkprintf(NULL, data, args);
    vkprintf(stream, data, args);
    va_end(args);
    //putchar('y');
}

char *write_int(char *buffer, int x) {
    if(x < 0) {
        *buffer = '-';
        buffer++;
        return write_int(buffer, -x);
    } else {
        if (x >= 10) 
            buffer = write_int(buffer, x / 10);
        *buffer = '0' + (x % 10);
        return buffer + 1;
    }
}
