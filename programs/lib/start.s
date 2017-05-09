.code32 

.section .text
.global _start
_start:
    nop
    nop
    nop
    nop
    nop
    call lib_init
    push $0x88001000
    call main
    call exit
    jmp _start
