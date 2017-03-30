#ifndef MEMORY_H
#define MEMORY_H
#include "int.h"
extern u32 end;

u32 kmalloc(u32 size);
u32 kmalloc_a(u32 size);
u32 kmalloc_p(u32 size, u32 *phys);
u32 kmalloc_3(u32 size, int align, u32 *phys);
// Start of the free space used for this kmalloc
#endif /* MEMORY_H */

