#include "isr.h"
#include "printing.h"
#include "io.h"
#include "keyboard.h"
#include "kernel.h"
#include "paging.h"

void test() {
    putchar('.');
}

void schedule() {
    //TODO
}

void print_reg(registers_t *x) {
    kprintf("Registres : eax %d, ebx %d, ecx %d, edx %d, esp %d, \
                         ebp %d, esi %d, edi %d.\n",
            x->eax, x->ebx, x->ecx, x->edx, x->esp, x->ebp, x->esi, x->edi);
}

void syscall(u32 id, context_t *context) {
    registers_t *regs = &(context->regs);
    //stack_state_t *stack = &(context->stack);
    
    kprintf("Caught syscall %d \n", id);
    print_reg(regs);
    
    picosyscall(context);
    
}

void print_stack(stack_state_t *x) {
    kprintf("ss %d, eip %d\n", x->ss, x->eip);
}

void isr_handler(u32 id, context_t *context) {
    //TODO Gérer les interruptions importantes !
    registers_t *regs = &(context->regs);
    stack_state_t *stack = &(context->stack);
    
    kprintf("Caught interruption %d, error code %d\n", id, context->err_code);
    print_reg(regs);
    print_stack(stack);
    
    if (id == 13) {
        kprintf("General Protection Fault, must die.");
        asm("hlt");
    }
    else if (id == 14) {
        page_fault(context);
    }
}

void irq_handler(u32 id, context_t *context) {
    if (id == 0) { // Timer
        picotimer(context);
    }
    else if(id == 1) { // Keyboard
        while ((inportb(0x64) & 0x01) == 0);
        provideKeyEvent(inportb(0x60));
    }
    else {
        kprintf("Received an IRQ with id %d.\n", id);
    }
    // Informs the PIC we are done with this IRQ
    outportb(0x20,0x20);
    
    if (id > 7) //Only tells the slave PIC for IRQ 8 .. IRQ 15
        outportb(0xA0,0x20);
}

