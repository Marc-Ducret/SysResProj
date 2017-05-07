#include "isr.h"
#include "printing.h"
#include "io.h"
#include "keyboard.h"
#include "kernel.h"
#include "paging.h"
#include "disk.h"
void test() {
    putchar('.');
}

void schedule() {
    //TODO
}

void print_reg(registers_t *x) {
    kprintf("Registres : eax %x, ebx %x, ecx %x, edx %x, esp %x, \
                         ebp %x, esi %x, edi %x.\n",
            x->eax, x->ebx, x->ecx, x->edx, x->esp, x->ebp, x->esi, x->edi);
}

void print_stack(stack_state_t *x) {
    kprintf("ss %x, esp %x, eflags %x, cs %x, eip %x\n", 
        x->ss, x->useresp, x->eflags, x->cs, x->eip);
}

void syscall(u32 id, context_t *context) {
    id = id;
    /*registers_t *regs = &(context->regs);
    stack_state_t *stack = &(context->stack);
    
    kprintf("Caught syscall %d \n", id);
    print_reg(regs);
    print_stack(stack);*/
    
    picosyscall(context);
}

void isr_handler(u32 id, context_t *context) {
    //TODO Gérer les interruptions importantes !
    registers_t *regs = &(context->regs);
    stack_state_t *stack = &(context->stack);
    
    u8 print = 1;
    u8 die = 0;
    
    if (id == 13) {
        kprintf("General Protection Fault, must die.");
        die = 1;
    } else if (id == 14) {
        if(page_fault(context)) die = 1;
        else print = 0;
    }
    if(print) {
        kprintf("Caught interruption %d, error code %x\n", id, context->err_code);
        print_reg(regs);
        print_stack(stack);
    }
    if(die) asm("hlt");
}

void irq_handler(u32 id, context_t *ctx) {
    if (id == 0) { // Timer
        picotimer(ctx);
    }
    else if(id == 1) { // Keyboard
        while ((inportb(0x64) & 0x01) == 0);
        provideKeyEvent(inportb(0x60));
    }
    else if (id == 14) { // HDD
        //kprintf("HDD Interrupt\n");
    }
    else if (id == 7) {
        // Strange IRQ happening (with bochs, not QEMU).
        // It seems like if no one on Internet could figure out why.
        outportb(0x20, 0x0B);
        u8 irr = inportb(0x20);
        if (!irr)
            return;
    }
    else {
        kprintf("Received an IRQ with id %d.\n", id);
    }
    // Informs the PIC we are done with this IRQ
    outportb(0x20,0x20);
    
    if (id > 7) //Only tells the slave PIC for IRQ 8 .. IRQ 15
        outportb(0xA0,0x20);
}

