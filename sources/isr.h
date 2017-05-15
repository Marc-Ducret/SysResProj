#ifndef ISR_H
#define ISR_H

char *exceptions_msg[32] = {
    "Divide-by-zero Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved", 
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Security Exception",
    "Reserved"
};

#endif /* ISR_H */

