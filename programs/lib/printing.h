#ifndef PRINTING_H
#define PRINTING_H

#include "int.h"
#include "lib.h"
#include "stream.h"

#define ESC_SEQ_START "\e["
#define ESC_SEQ_END "|"

#define ESC_LINE(y) y
#define ESC_COLUMN(x) (x + 25)
#define ESC_BACK_NO_RET 105
#define ESC_FORW_NO_RET 106
#define ESC_UP 107
#define ESC_DOWN 108
#define ESC_BACK 109
#define ESC_FORW 110
#define ESC_FG(c) (c + 128)
#define ESC_BG(c) (c + 144)
#define ESC_PREV_FG 160
#define ESC_PREV_BG 161
#define ESC_KILL 162
#define ESC_CLEAR 163
#define ESC_SCROLL_UP 164
#define ESC_SCROLL_DOWN 165
#define ESC_ERASE 166

void printf(const char* data, ...);
void fprintf(sid_t sid, const char* data, ...);
void fprintf_esc_seq(u8 esc_value, sid_t sid);
void esc_seq(u8 esc_value);
void print_time(rtc_time_t *t);
#endif

