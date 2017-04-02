#ifndef LIB_H
#define LIB_H
#include "int.h"
void *memcpy(char *dst, char *src, int n);
void memset(char *dst, char src, u32 len);
int strEqual(char *strA, char *strB);

#endif
