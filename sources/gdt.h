/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gdt.h
 * Author: martin
 *
 * Created on 15 mars 2017, 15:12
 */

#ifndef GDT_H
#define GDT_H
#include "int.h"

struct tss {
	u16 previous_task, __previous_task_unused;
	u32 esp0;
	u16 ss0, __ss0_unused;
	u32 esp1;
	u16 ss1, __ss1_unused;
	u32 esp2;
	u16 ss2, __ss2_unused;
	u32 cr3;
	u32 eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	u16 es, __es_unused;
	u16 cs, __cs_unused;
	u16 ss, __ss_unused;
	u16 ds, __ds_unused;
	u16 fs, __fs_unused;
	u16 gs, __gs_unused;
	u16 ldt_selector, __ldt_sel_unused;
	u16 debug_flag, io_map;
} __attribute__ ((packed));

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

struct	idtr {
    u16	limite;
    u32	base;
} __attribute__	((packed));

struct idtdesc {
    u16 offset0_15;
    u16 select;
    u8 zeros;
    u8 type;
    u16 offset16_31;
} __attribute__ ((packed));

#define GDTBASE 0x00000800
#define GDTSIZE 0xFF

#define IDTBASE 0x00000000
#define IDTSIZE 0xFF
#define INTGATE 0x8E //Defines the type for an interruption gate
#define TRAPGATE 0x8F //Defines the type for a trap gate
#define TRAPGATE_USER 0xEF //Allows the user to use such interruptions

void init_gdt();
void init_idt();
void init_pic();
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void asm_syscall();
#endif /* GDT_H */

