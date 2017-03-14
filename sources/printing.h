/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   printing.h
 * Author: martin
 *
 * Created on 12 mars 2017, 17:07
 */

#ifndef PRINTING_H
#define PRINTING_H

void terminal_initialize();
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void putchar(char c);
void terminal_writestring(const char* data);
void putint(int i);
void kprintf(const char* data, ...);

#endif /* PRINTING_H */

