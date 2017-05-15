#include "isr.h"
#include "printing.h"
#include "io.h"
#include "keyboard.h"
#include "kernel.h"
#include "paging.h"
#include "disk.h"
#include "timer.h"

void print_reg(registers_t *x) {
    fprintf(stderr, "Registers State: eax %x, ebx %x, ecx %x, edx %x, esp %x,\
ebp %x, esi %x, edi %x.\n",
            x->eax, x->ebx, x->ecx, x->edx, x->esp, x->ebp, x->esi, x->edi);
}

void print_stack(stack_state_t *x) {
    fprintf(stderr, "Stack State: ss %x, esp %x, eflags %x, cs %x, eip %x\n", 
        x->ss, x->useresp, x->eflags, x->cs, x->eip);
}

void syscall(u32 id, context_t *context) {
    id = id; // To avoid compilation warning.
    picosyscall(context);
}

void isr_handler(u32 id, context_t *context) {
    //TODO Gérer les interruptions importantes !
    registers_t *regs = &(context->regs);
    stack_state_t *stack = &(context->stack);
    state *s = get_global_state();
    
    if (id == 14) {
        page_fault(context);
        fprintf(stderr, "Process %d was killed because of this Page Fault\n");
    }
    else {
        fprintf(stderr, "Process %d was killed because of :\n%s\n",
                s->curr_pid, exceptions_msg[id]);
        print_reg(regs);
        print_stack(stack);
    }
    flush(stderr);
    if(s->curr_pid >= 0) {
        kill_process(s->curr_pid);
        reorder(s);
    } else asm("hlt");
}

void irq_handler(u32 id, context_t *ctx) {
    if (id == 0) { // Timer
        current_time.mseconds++;
        picotimer(ctx);
    }
    else if(id == 1) { // Keyboard
        while ((inportb(0x64) & 0x01) == 0);
        provideKeyEvent(inportb(0x60));
    }
    else if (id == 14) { 
        // HDD
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

