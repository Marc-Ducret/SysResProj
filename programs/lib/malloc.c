#include "malloc.h"

malloc_header_t *first_block = NULL;

void *expend_heap(u32 size) {
    return NULL; //TODO do
}

void shrink_heap(u32 size) {

}

void *malloc(u32 size) {
    size += sizeof(malloc_header_t);
    malloc_header_t *prev_block;
    for(malloc_header_t *block = first_block; block; block = block->next_block) {
        if((u32) block->next_block - block->size - (u32) block >= size) {
            malloc_header_t *new_block = (malloc_header_t *) ((u32)block + block->size);
            new_block->prev_block = block;
            new_block->next_block = block->next_block;
            new_block->size = size;
            next_block->prev_block = new_block;
            block->next_block = new_block;
            return (void *) new_block + sizeof(malloc_header_t);
        }
        prev_block = block;
    }
    malloc_header_t *new_block = (malloc_header_t *) expand_heap(size);
    new_block->size = size;
    new_block->next_block = NULL;
    if(prev_block) {
        prev_block->next_block = new_block;
        new_block->prev_block = prev_block;
        //TODO CHECK prev_block + prev_block->size = new_block
    } else {
        new_block->prev_block = NULL;
        first_block = new_block;
    }
    return (void *) new_block + sizeof(malloc_header_t);
}

void free(void *allocated) {
    malloc_header_t *block = (malloc_header_t*) allocated - 1;
    if(block->prev_block) {
        block->prev_block->next_block = block->next_block;
        if(block->next_block)
            block->next_block->prev_block = block->prev_block;
        else {
            shrink_heap(block->size);
        }
    } else {
        shrink_heap(block->size);
        first_block = NULL;
    }
}
