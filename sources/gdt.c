#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed));


void init_descriptor() {
    //Creates the simple following GDT at address 0x800.
    struct gdt {
        unsigned int address;
        unsigned short size;
    } __attribute__((packed));
    
    struct gdt *gdt;
    
    uint64_t *ptr = (uint64_t *) 0x800;
    
    gdt->address = 0x800;
    gdt->size = 24;

    uint64_t descriptor0 = 0x0;

    // Which ones are CORRECTS ?
    uint64_t descriptor1 = 0x00CF9A000000FFFF; //Code segment 
    uint64_t descriptor2 = 0x00CF92000000FFFF; //Data segment
    //uint64_t descriptor1 = 0xFFFF00000092CF00; //Code segment 
    //uint64_t descriptor2 = 0xFFFF0000009ACF00; // Data segment

	//Filling the GDT
    ptr[0] = descriptor0;
    ptr[1] = descriptor1;
    ptr[2] = descriptor2;
	
	//Some debug
    uint32_t x;
    asm volatile("movw	%%cs, (%0)":"=r" (x):);
    putint(x);
    kprintf("\n");

    //asm("lgdtl (%0)":: "r" (gdt)); // Would load the new gdt..
   
    //Load new segments
    asm(" movw	$0x18,	%ax \n \
          movw	%ax,	%ds \n");/*
          movw	%ax,	%fs \n \
          movw	%ax,	%es \n \
          movw	%ax,	%gs \n ");*/

    //asm("ljmp	$0x0, $next1 \n \
          next1: \n"); //Supposed to jump to update cs..
	//Fun fact : the affectation of %ds changes %cs value..
    kprintf("Whaouh \n");
    asm volatile("movw	%%cs, (%0)":"=r" (x):);
    putint(x);
    kprintf("\n");

    asm volatile("movw	%%cs, (%0)":"=r" (x):);
    putint(x); // and then cs changes again to 0..
	// By the way, zero is supposed to be always invalid.
    kprintf("\n");
    kprintf("Did it !");
}
