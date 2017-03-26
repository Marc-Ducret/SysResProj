#ifndef ISR_H
#define ISR_H
#include "int.h"

typedef struct registers
{
   u32 gs, fs, es, ds;                  // Data segment selector
   u32 eax, ebx, ecx, edx, esp, ebp, esi, edi; // Pushed by pusha.
} __attribute__ ((packed)) registers_t;

typedef struct stack_state {
   u32 eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} __attribute__ ((packed)) stack_state_t;

#endif /* ISR_H */

