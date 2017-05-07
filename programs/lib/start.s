.code32 

.section .text
.global _start
_start:
    nop
    nop
    nop
    nop
    nop
    push $1
    push $0x88001000
    call main
    jmp _start
