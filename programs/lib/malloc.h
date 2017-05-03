#ifndef MALLOC_H
#define MALLOC_H

#include "lib.h"

typedef struct malloc_header malloc_header_t;

struct malloc_header {

    u32 size;
    malloc_header_t *prev_block, *next_block;
};

void *malloc(u32 size);

#endif
