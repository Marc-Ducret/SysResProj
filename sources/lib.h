#ifndef LIB_H
#define LIB_H
#include "int.h"
void *memcpy(void *dst, void *src, u32 n);
void memset(void *dst, u8 src, u32 len);
int strEqual(char *strA, char *strB);

#endif
