#ifndef LIB_H
#define LIB_H
#include "int.h"
void *memcpy(u8 *dst, u8 *src, u32 n);
void memset(u8 *dst, u8 src, u32 len);
int strEqual(char *strA, char *strB);

#endif
