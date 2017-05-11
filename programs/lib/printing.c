#include "printing.h"

int putchar(char c, sid_t sid) {
    return stream_putchar(c, sid);
}

void putint(int i, sid_t sid) {
    if(i < 0) {
        putchar('-', sid);
        putint(-i, sid);
    } else {
        if(i >= 10) putint(i / 10, sid);
        putchar('0' + (i % 10), sid);
    }
}

void put_hex(u32 i, sid_t sid) {
    if(i >= 16) put_hex(i >> 4, sid);
    u32 number = i % 16;
    putchar((number < 10) ? ('0' + number) : ('A' + number - 10), sid);
}

void put_string(char *s, sid_t sid) {
    u32 datalen = strlen(s);
    for ( u32 i = 0; i < datalen; i++ )
        putchar(s[i], sid);
}

void esc_seq(u8 esc_value) {
    fprintf_esc_seq(esc_value, STDOUT);
}

void fprintf_esc_seq(u8 esc_value, sid_t sid) {
    fprintf(sid, "%s%c%s", ESC_SEQ_START, esc_value, ESC_SEQ_END);
}

void vkprintf(sid_t sid, const char* data, va_list args) {    
    char c = *data;
    while (c != 0) {
        if (c == '%') {
            data ++;
            c = *data;
            int nb;
            char *s;
            char ch;
            u8 color;
            
            switch (c) {
                case 'd':
                    nb = va_arg(args, int);
                    putint(nb, sid);
                    break;
                
                case 's':
                    s = va_arg(args, char*);
                    put_string(s, sid);
                    break;
                
                case 'x':
                    put_string("0x", sid);
                
                case 'h':
                    nb = va_arg(args, unsigned int);
                    put_hex(nb, sid);
                    break;
                
                case 'c':
                    nb = va_arg(args, int);
                    ch = (char) nb;
                    putchar(ch, sid);
                    break;
                    
                case 'f':
                    if (*(++data) != 'g') {
                        data--;
                        break;
                    }
                    nb = va_arg(args, int);
                    color = (u8) nb;
                    fprintf_esc_seq(ESC_FG(color), sid);
                    break;
                    
                case 'b':
                    if (*(++data) != 'g') {
                        data--;
                        break;
                    }
                    nb = va_arg(args, int);
                    color = (u8) nb;
                    fprintf_esc_seq(ESC_BG(color), sid);
                    break;
                    
                case 'p':
                    data++;
                    if (*data == 'f') {
                        if (*(++data) != 'g') {
                            data--;
                            break;
                        }
                        fprintf_esc_seq(ESC_PREV_FG, sid);
                        break;
                    }
                    if (*data == 'b') {
                        if (*(++data) != 'g') {
                            data--;
                            break;
                        }
                        fprintf_esc_seq(ESC_PREV_BG, sid);
                        break;
                    }
                    if (*data == 'c') {
                        fprintf_esc_seq(ESC_PREV_BG, sid);
                        fprintf_esc_seq(ESC_PREV_FG, sid);
                        break;
                    }
                    data--;
                    break;
                    
                default:
                    break;
            }
        } else {
            putchar(c, sid);
        }
        
        data ++;
        c = *data;
    }    
}

void printf(const char* data, ...) {
    va_list args;
    va_start(args, data);
    vkprintf(STDOUT, data, args);
    va_end(args);
}

void fprintf(sid_t sid, const char *data, ...) {
    va_list args;
    va_start(args, data);
    vkprintf(sid, data, args);
    va_end(args);
}
