#include "malloc.h"

malloc_header_t *first_block;

void *sbrk(int size) {
    return NULL; //TODO do
}

void *malloc(u32 size) {
    size += sizeof(malloc_header_t);
    malloc_header_t *prev_block;
    for(malloc_header_t *block = first_block; block; block = block->next_block) {
        if((u32) block->next_block - block->size - (u32) block >= size) {
            malloc_header_t *new_block = (malloc_header_t *) ((u32)block + block->size);
            new_block->next_block = block->next_block;
            new_block->size = size;
            block->next_block = new_block;
            return (void *) new_block + sizeof(malloc_header_t);
        }
        prev_block = block;
    }
    malloc_header_t *new_block = (malloc_header_t *) sbrk(size);
    new_block->size = size;
    new_block->next_block = NULL;
    if(prev_block) prev_block->next_block = new_block;
    return (void *) new_block + sizeof(malloc_header_t);
}

void free(void *allocated) {
    //TODO add prev block
}
