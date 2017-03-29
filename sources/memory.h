#ifndef MEMORY_H
#define MEMORY_H
#include "int.h"

u32 kmalloc(u32 size);
u32 kmalloc_a(u32 size);
u32 kmalloc_p(u32 size, u32 *phys);
u32 kmalloc_3(u32 size, int align, u32 *phys);
// Start of the free space used for this kmalloc
u32 next = 0x00000000;
#endif /* MEMORY_H */

