#include "memory.h"
#include <stddef.h>

u32 free_address = &end;

u32 kmalloc_3(u32 size, int align, u32 *phys) {
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
    
    u32 tmp = size;
    free_address += size;
    
    return tmp;
}

u32 kmalloc(u32 size) {
    return kmalloc_3(size, 0, NULL);
}

u32 kmalloc_a(u32 size) {
    return kmalloc_3(size, 1, NULL);
}

u32 kmalloc_p(u32 size, u32 *phys) {
    return kmalloc_3(size, 0, phys);
}


//TODO Frame allocation !