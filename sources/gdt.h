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

#define GDTBASE 0x00000800
#define GDTSIZE 0xFF
void init_gdt();
#endif /* GDT_H */

