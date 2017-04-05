#include "io.h"
u8 inportb (u16 _port) {
    u8 rv;
    asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (u16 _port, u8 _data) {
    asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

u16 inportw (u16 _port) {
    u16 rv;
    asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportw (u16 _port, u16 _data) {
    asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

void wait_key(u8 k) {
    while(inportb(0x60) != k) {}
}
