#ifndef LIB_H
#define LIB_H
#include "int.h"
void *memcpy(void *dst, void *src, u32 n);
void memset(void *dst, u8 src, u32 len);
int strEqual(char *strA, char *strB);
void strCopy(char *src, char* dest);
u32 strlen(const char* str);
void assert(int condition);
int min(int a, int b);
int max(int a, int b);
u32 umax(u32 a, u32 b);
u32 umin(u32 a, u32 b);
#endif
