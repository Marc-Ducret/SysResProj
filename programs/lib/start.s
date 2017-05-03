.code32 

.section .text
.global _start
_start:
    nop
    nop
    nop
    nop
    nop
    call main
    jmp _start
