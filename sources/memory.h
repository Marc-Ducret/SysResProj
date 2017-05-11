#ifndef MEMORY_H
#define MEMORY_H
#include "int.h"
extern u32 end;
volatile u32 free_address, kernel_mem_end;

typedef struct malloc_header malloc_header_t;

struct malloc_header {

    u32 size;
    malloc_header_t *prev_block, *next_block;
};

void *malloc(u32 size);
void free(void *allocated);
void init_malloc();
void print_malloc();
// Start of the free space used for this kmalloc
#endif /* MEMORY_H */

