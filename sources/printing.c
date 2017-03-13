#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "args.h"

static const uint8_t COLOR_BLACK = 0;
static const uint8_t COLOR_BLUE = 1;
static const uint8_t COLOR_GREEN = 2;
static const uint8_t COLOR_CYAN = 3;
static const uint8_t COLOR_RED = 4;
static const uint8_t COLOR_MAGENTA = 5;
static const uint8_t COLOR_BROWN = 6;
static const uint8_t COLOR_LIGHT_GREY = 7;
static const uint8_t COLOR_DARK_GREY = 8;
static const uint8_t COLOR_LIGHT_BLUE = 9;
static const uint8_t COLOR_LIGHT_GREEN = 10;
static const uint8_t COLOR_LIGHT_CYAN = 11;
static const uint8_t COLOR_LIGHT_RED = 12;
static const uint8_t COLOR_LIGHT_MAGENTA = 13;
static const uint8_t COLOR_LIGHT_BROWN = 14;
static const uint8_t COLOR_WHITE = 15;

uint8_t make_color(uint8_t fg, uint8_t bg)
{
	return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

size_t strlen(const char* str)
{
	size_t ret = 0;
	while ( str[ret] != 0)
		ret++;
	return ret;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 24;

size_t terminal_row=1;
size_t terminal_column=1;
uint8_t terminal_color=1;
uint16_t* terminal_buffer=1;

void terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_BLUE, COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for ( size_t y = 0; y < VGA_HEIGHT; y++ )
		for ( size_t x = 0; x < VGA_WIDTH; x++ )
		{
			const size_t index = y * VGA_WIDTH + x;
			//terminal_buffer[index] = make_vgaentry(x % 2 == 0 ? '.' : ' ', terminal_color);
                        terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void putchar(char c)
{
        if (c == '\n')
        {
            while (terminal_column < VGA_WIDTH)
            {
                terminal_putentryat(' ', make_color(COLOR_RED, COLOR_GREEN), terminal_column, terminal_row);
                terminal_column ++;
            }
            terminal_column --;
        }
	else 
        {
            terminal_putentryat(c, make_color(COLOR_RED, COLOR_GREEN), terminal_column, terminal_row);
        }
        
	if ( ++terminal_column == VGA_WIDTH )
	{
		terminal_column = 0;
		if ( ++terminal_row == VGA_HEIGHT )
		{
			terminal_row = 0;
		}
	}
}

void putint(int i) {
    if(i < 0) 
    {
        putchar('-');
        putint(-i);
    }
    else {
        if(i >= 10) putint(i / 10);
        putchar('0' + (i % 10));
    }
}

void terminal_writestring(const char* data) {
	size_t datalen = strlen(data);
	for ( size_t i = 0; i < datalen; i++ )
		putchar(data[i]);
}

void print_string(char *s)
{
    terminal_writestring(s);
}

void kprintf(const char* data, ...) {
    va_list args;
    va_start(args, data);
    
    char c = *data;
    while (c != 0)
    {
        if (c == '%')
        {
            data ++;
            c = *data;
            int nb;
            char *s;
            
            switch (c)
            {
                case 'd':
                    nb = va_arg(args, int);
                    putint(nb);
                    break;
                
                case 's':
                    s = va_arg(args, char*);
                    print_string(s);
                default:
                    break;
            }
        }
        else
        {
            putchar(c);
        }
        
        data ++;
        c = *data;
    }
    terminal_writestring(data);
    
    va_end(args);
}

