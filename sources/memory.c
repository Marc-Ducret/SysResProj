#include "memory.h"
#include <stddef.h>
#include "printing.h"
#define HEAP_SIZE 0x2000000

volatile u32 free_address = (u32) &end;
volatile u32 kernel_mem_end = (u32) &end + HEAP_SIZE + 1;

void *start_heap;
void *heap_pointer;
malloc_header_t *first_block;

void print_malloc() {
    kprintf("heap: %x-%x\n", start_heap, heap_pointer);
}

void init_malloc() {
    kprintf("Init malloc\n");
    start_heap = free_address;
    heap_pointer = start_heap;
    first_block = NULL;
    malloc(0);
}

void *expand_heap(int size) {
    void *h = heap_pointer;
    if(size) {
        heap_pointer += size;
        if(heap_pointer >= start_heap + HEAP_SIZE) {
            kprintf("KERNEL HEAP OVERFLOW\n");
            asm("cli\nhlt");
        }
    }
    return h;
}

void shrink_heap(int size) {
    heap_pointer -= size;
}

void *over_align(void *p) {
    return (void*) (((u32)p-1) | 0xFFF) + 1;
}

void *malloc(u32 size) {
    size += sizeof(malloc_header_t);
    malloc_header_t *prev_block = NULL;
    for(malloc_header_t *block = first_block; block; block = block->next_block) {
        void *potential_addr = over_align((void *)block + block->size + sizeof(malloc_header_t))
                                                                       - sizeof(malloc_header_t);
        if(block->next_block && (u32) block->next_block >= (u32) potential_addr + size) {
            malloc_header_t *new_block = (malloc_header_t *) (potential_addr);
            memset(new_block, 0, size);
            //kprintf("alloc at %x [s=%x] between %x and %x\n", new_block, size, (void*)block+block->size, block->next_block);
            new_block->prev_block = block;
            new_block->next_block = block->next_block;
            new_block->size = size;
            block->next_block->prev_block = new_block;
            block->next_block = new_block;
            if(((u32) new_block + sizeof(malloc_header_t)) & 0xFFF) {
                kprintf("Malloc align failure\n");
                asm("cli\nhlt");
            }
            return (void *) new_block + sizeof(malloc_header_t);
        }
        prev_block = block;
    }
    malloc_header_t *new_block = (malloc_header_t *) (over_align(heap_pointer   + sizeof(malloc_header_t))
                                                                                 - sizeof(malloc_header_t));
    expand_heap((u32) new_block + size - (u32) heap_pointer);
    if(!new_block) return NULL;
    memset(new_block, 0, size);
    new_block->size = size;
    new_block->next_block = NULL;
    if(prev_block) {
        prev_block->next_block = new_block;
        new_block->prev_block = prev_block;
    } else {
        new_block->prev_block = NULL;
        first_block = new_block;
    }
    if(((u32) new_block + sizeof(malloc_header_t)) & 0xFFF) {
        kprintf("Malloc align failure\n");
        asm("cli\nhlt");
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
            shrink_heap((u32) block + block->size - (u32) block->prev_block - block->prev_block->size);
        }
    } else {
        kprintf("Malloc free first block\n");
        asm("cli\nhlt");
        shrink_heap(block->size);
        first_block = NULL;
    }
}
