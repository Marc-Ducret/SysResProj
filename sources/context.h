#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct registers
{
   u32 gs, fs, es, ds;                  // Data segment selector
   u32 eax, ebx, ecx, edx, esp, ebp, esi, edi; // Pushed by pusha.
} __attribute__ ((packed)) registers_t;

typedef struct stack_state 
{
   u32 eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__ ((packed)) stack_state_t;

typedef struct context_t 
{
    registers_t regs;
    u32 err_code;
    stack_state_t stack;
} __attribute((packed)) context_t;

#endif
