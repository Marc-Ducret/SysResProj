#ifndef INT_H
#define INT_H

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;

#define BREAKPOINT asm volatile("xchg %bx, %bx")
#endif
