#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "printing.h"
#include "gdt.h"
#include "lib.h"

//u32 x;
// code pour afficher rip
/*asm("jmp next3 \n \
         next2: pop x \n \
         push x \n \
         ret \n \
         next3: \n \
         call next2 \n \
         ");
    putint(x);*/

struct tss default_tss;
struct gdtdesc kgdt[GDTSIZE];
struct gdtr kgdtr;
struct idtdesc kidt[IDTSIZE];
struct gdtr idtr;

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
    // Initialize the default tss
    default_tss.debug_flag = 0x00;
    default_tss.io_map = 0x00;
    default_tss.esp0 = 0x1FFF0;
    default_tss.ss0 = 0x18;
                                
    /* initialize gdt segments */
    init_gdt_desc(0x0, 0x0, 0x0, 0x0, &kgdt[0]);
    init_gdt_desc(0x00, 0xFFFFF, 0x9B, 0x0D, &kgdt[1]);  /* code */
    init_gdt_desc(0x00, 0xFFFFF, 0x93, 0x0D, &kgdt[2]);  /* data */
    init_gdt_desc(0x0, 0x0, 0x97, 0x0D, &kgdt[3]);      /* stack */

    init_gdt_desc(0x0, 0xFFFFF, 0xFF, 0x0D, &kgdt[4]);  /* ucode */
    init_gdt_desc(0x0, 0xFFFFF, 0xF3, 0x0D, &kgdt[5]);  /* udata */
    init_gdt_desc(0x0, 0x0, 0xF7, 0x0D, &kgdt[6]);      /* ustack */

    init_gdt_desc((u32) & default_tss, 0x67, 0xE9, 0x00, &kgdt[7]); /* descripteur de tss */

    /* initialize the gdtr structure */
    kgdtr.limite = GDTSIZE * 8;
    kgdtr.base = GDTBASE;

    /* copy the gdtr to its memory area */
    memcpy((char *) kgdtr.base, (char *) kgdt, kgdtr.limite);

    /* load the gdtr registry */
    asm("lgdtl (kgdtr)");
    
    /* initiliaze the segments */
    asm("   movw $0x10, %ax \n \
            movw %ax, %ds   \n \
            movw %ax, %es   \n \
            movw %ax, %fs   \n \
            movw %ax, %gs   \n \
            ljmp $0x08, $next   \n \
            next:       \n");
    kprintf("GDT initialized\n");
}

void init_idt_desc(u8 num, u16 select, u32 offset, u8 type) {
    struct idtdesc *desc = &(kidt[num]);
    desc->offset0_15 = (offset & 0xffff);
    desc->offset16_31 = (offset & 0xffff0000) >> 16;
    desc->zeros = 0;
    desc->type = type;
    desc->select = select;
}

void init_idt(void) {
    // Adding the many isr
    init_idt_desc(0, 0x08, (u32) isr0, INTGATE);
    init_idt_desc(1, 0x08, (u32) isr1, INTGATE);
    init_idt_desc(2, 0x08, (u32) isr2, INTGATE);
    init_idt_desc(3, 0x08, (u32) isr3, INTGATE);
    init_idt_desc(4, 0x08, (u32) isr4, INTGATE);
    init_idt_desc(5, 0x08, (u32) isr5, INTGATE);
    init_idt_desc(6, 0x08, (u32) isr6, INTGATE);
    init_idt_desc(7, 0x08, (u32) isr7, INTGATE);
    init_idt_desc(8, 0x08, (u32) isr8, INTGATE);
    init_idt_desc(9, 0x08, (u32) isr9, INTGATE);
    init_idt_desc(10, 0x08, (u32) isr10, INTGATE);
    init_idt_desc(11, 0x08, (u32) isr11, INTGATE);
    init_idt_desc(12, 0x08, (u32) isr12, INTGATE);
    init_idt_desc(13, 0x08, (u32) isr13, INTGATE);
    init_idt_desc(14, 0x08, (u32) isr14, INTGATE);
    init_idt_desc(15, 0x08, (u32) isr15, INTGATE);
    init_idt_desc(16, 0x08, (u32) isr16, INTGATE);
    init_idt_desc(17, 0x08, (u32) isr17, INTGATE);
    init_idt_desc(18, 0x08, (u32) isr18, INTGATE);
    init_idt_desc(19, 0x08, (u32) isr19, INTGATE);
    init_idt_desc(20, 0x08, (u32) isr20, INTGATE);
    init_idt_desc(21, 0x08, (u32) isr21, INTGATE);
    init_idt_desc(22, 0x08, (u32) isr22, INTGATE);
    init_idt_desc(23, 0x08, (u32) isr23, INTGATE);
    init_idt_desc(24, 0x08, (u32) isr24, INTGATE);
    init_idt_desc(25, 0x08, (u32) isr25, INTGATE);
    init_idt_desc(26, 0x08, (u32) isr26, INTGATE);
    init_idt_desc(27, 0x08, (u32) isr27, INTGATE);
    init_idt_desc(28, 0x08, (u32) isr28, INTGATE);
    init_idt_desc(29, 0x08, (u32) isr29, INTGATE);
    init_idt_desc(30, 0x08, (u32) isr30, INTGATE);
    init_idt_desc(31, 0x08, (u32) isr31, INTGATE);
    
    init_idt_desc(32, 0x08, (u32) irq0, INTGATE); //Timer
    init_idt_desc(33, 0x08, (u32) irq1, INTGATE); //Keyboard
    init_idt_desc(34, 0x08, (u32) irq2, INTGATE);
    init_idt_desc(35, 0x08, (u32) irq3, INTGATE);
    init_idt_desc(36, 0x08, (u32) irq4, INTGATE);
    init_idt_desc(37, 0x08, (u32) irq5, INTGATE);
    init_idt_desc(38, 0x08, (u32) irq6, INTGATE);
    init_idt_desc(39, 0x08, (u32) irq7, INTGATE);
    init_idt_desc(40, 0x08, (u32) irq8, INTGATE);
    init_idt_desc(41, 0x08, (u32) irq9, INTGATE);
    init_idt_desc(42, 0x08, (u32) irq10, INTGATE);
    init_idt_desc(43, 0x08, (u32) irq11, INTGATE);
    init_idt_desc(44, 0x08, (u32) irq12, INTGATE);
    init_idt_desc(45, 0x08, (u32) irq13, INTGATE);
    init_idt_desc(46, 0x08, (u32) irq14, INTGATE);
    init_idt_desc(47, 0x08, (u32) irq15, INTGATE);
    
    init_idt_desc(128, 0x08, (u32) asm_syscall, TRAPGATE);

    idtr.limite = IDTSIZE * 8;
    idtr.base = IDTBASE;
    
    /* copy the idtr to its memory area */
    memcpy((char *) idtr.base, (char *) kidt, idtr.limite);
    
    asm("lidtl (idtr)");
    kprintf("IDT initialized\n");
}

void init_pic(void)
{
    /* Initialization of ICW1 */
    outportb(0x20, 0x11);
    outportb(0xA0, 0x11);

    /* Initialization of ICW2 */
    outportb(0x21, 0x20);	/* start vector of master PIC = 32 */
    outportb(0xA1, 0x28);	/* start vector of slave PIC = 40 */
    //outportb(0xA1, 0x70);	/* start vector of slave PIC = 112 */

    /* Initialization of ICW3 */
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);

    /* Initialization of ICW4 */
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);

    /* mask interrupts */
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
    
    kprintf("PIC initialized\n");
}
