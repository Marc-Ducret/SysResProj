#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "printing.h"

void* memcpy(char *dst, char *src, int n) {
    char *p = dst;
    while (n--)
        *dst++ = *src++;
    return p;
}

struct gdtr {
    unsigned short limite;
    unsigned int base;
} __attribute__ ((packed));

struct gdtdesc {
    unsigned short lim0_15;
    unsigned short base0_15;
    unsigned char base16_23;
    unsigned char acces;
    unsigned char lim16_19:4;
    unsigned char other:4;
    unsigned char base24_31;
} __attribute__ ((packed));

#define GDTBASE 0x00000800
#define GDTSIZE 0xFF

struct gdtdesc kgdt[GDTSIZE];
struct gdtr    kgdtr;

void init_gdt_desc(unsigned int base, unsigned int limite, unsigned char acces,
                 unsigned char other, struct gdtdesc *desc) {
    desc->lim0_15 = (limite & 0xffff);
    desc->base0_15 = (base & 0xffff);
    desc->base16_23 = (base & 0xff0000) >> 16;
    desc->acces = acces;
    desc->lim16_19 = (limite & 0xf0000) >> 16;
    desc->other = (other & 0xf);
    desc->base24_31 = (base & 0xff000000) >> 24;
    return;
}

void init_gdt(void) {
    /* initialize gdt segments */
    init_gdt_desc(0x0, 0x0, 0x0, 0x0, &kgdt[0]);
    init_gdt_desc(0x0, 0xFFFFF, 0x9B, 0x0D, &kgdt[1]);  /* code */
    init_gdt_desc(0x0, 0xFFFFF, 0x93, 0x0D, &kgdt[2]);  /* data */
    init_gdt_desc(0x0, 0x0, 0x97, 0x0D, &kgdt[3]);      /* stack */

    init_gdt_desc(0x0, 0xFFFFF, 0xFF, 0x0D, &kgdt[4]);  /* ucode */
    init_gdt_desc(0x0, 0xFFFFF, 0xF3, 0x0D, &kgdt[5]);  /* udata */
    init_gdt_desc(0x0, 0x0, 0xF7, 0x0D, &kgdt[6]);      /* ustack */

    //init_gdt_desc((unsigned int) & default_tss, 0x67, 0xE9, 0x00, &kgdt[7]); /* descripteur de tss */

    /* initialize the gdtr structure */
    kgdtr.limite = GDTSIZE * 8;
    kgdtr.base = GDTBASE;

    /* copy the gdtr to its memory area */
    memcpy((char *) kgdtr.base, (char *) kgdt, kgdtr.limite);

    /* load the gdtr registry */
    asm("lgdtl (kgdtr)");

    /* initiliaz the segments */
    asm("   movw $0x10, %ax \n \
            movw %ax, %ds   \n \
            movw %ax, %es   \n \
            movw %ax, %fs   \n \
            movw %ax, %gs   \n \
            ljmp $0x08, $next   \n \
            next:       \n");

    kprintf("GDT initialized\n");
}
