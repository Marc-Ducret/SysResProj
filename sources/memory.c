#include "memory.h"
#include <stddef.h>

volatile u32 free_address = (u32) &end;

void* kmalloc_3(u32 size, int align, u32 *phys) {
    //Simple linear malloc, whithout any free.
    //If align, then aligns with 0x1000 blocks.
    //If phys, then puts the physical address into *phys.
    
    if (align && (free_address & 0xFFF)) {
        //Need to be realigned
        free_address &= 0xFFFFF000;
        free_address += 0x00001000;
    }
    
    if (phys)
        *phys = free_address;
    
    void* tmp = (void*) free_address;
    free_address += size;

    return tmp;
}

void* kmalloc(u32 size) {
    return kmalloc_3(size, 0, NULL);
}

void* kmalloc_a(u32 size) {
    return kmalloc_3(size, 1, NULL);
}

void* kmalloc_p(u32 size, u32 *phys) {
    return kmalloc_3(size, 0, phys);
}


//TODO Frame allocation !
