#ifndef INT_H
#define INT_H

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
typedef u32 size_t;
typedef int ssize_t;
typedef int pid_t; 
#define BREAKPOINT asm volatile("xchg %bx, %bx")
#endif
