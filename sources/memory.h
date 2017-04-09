#ifndef MEMORY_H
#define MEMORY_H
#include "int.h"
extern u32 end;
volatile u32 free_address, kernel_mem_end;
void* kmalloc(u32 size);
void* kmalloc_a(u32 size);
void* kmalloc_p(u32 size, u32 *phys);
void* kmalloc_3(u32 size, int align, u32 *phys);
// Start of the free space used for this kmalloc
#endif /* MEMORY_H */

