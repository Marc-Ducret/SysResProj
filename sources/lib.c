#include "int.h"
#include "printing.h"

void *memcpy(void *dst, void *src, u32 n) {
    u8 *p = dst;
    while(n--)
        *(u8*)dst++ = *(u8*)src++;
    return p;
}

void memset(void *dst, u8 src, u32 len) {
    // Copy len times src from the address dst.
    while (len--) {
        *(u8*)dst++ = src;
    }
}

int strEqual(char *strA, char *strB) {
    int i = 0;
    while(strA[i] == strB[i]) {
        if(strA[i] == 0) return 1;
        i++;
    }
    return 0;
}

void strCopy(char *src, char* dest) {
    while (*src) {
        *dest ++ = *src ++;
    }
    *dest = 0;
}

u32 strlen(const char* str) {
    u32 ret = 0;
    while(str[ret] != 0)
        ret++;
    return ret;
}

void assert(int condition, char *msg) {
    if (!condition) {
        kprintf("Assertion Failure : %s\n", msg);
        for (;;) {
            asm("hlt");
        }
    }
}

int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a < b ? b : a;
}

u32 umax(u32 a, u32 b) {
    return a < b ? b : a;
}

u32 umin(u32 a, u32 b) {
    return a < b ? a : b;
}
