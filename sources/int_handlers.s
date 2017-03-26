.macro isr_error_code nb
.global isr\nb
isr\nb:
    cli
    pushr # Saves the registers
    push $\nb # Saves the number of interrupt
    call isr_handler
    add $0x4, %esp
    popr
    add $0x4, %esp
    sti
    iret
.endm

.macro isr_no_error_code nb
.global isr\nb
isr\nb:
    cli
    push $0 # Adds an error code
    pushr # Saves the registers
    push $\nb # Saves the number of interrupt
    call isr_handler
    add $4, %esp
    popr
    add $0x4, %esp
    sti
    iret
.endm

.macro irq nb
.global irq\nb
irq\nb:
    cli
    pushr
    push $\nb
    call irq_handler
    add $4, %esp
    popr
    sti
    iret
.endm

.macro pusha
    push %edi
    push %esi
    push %ebp
    push %esp
    push %edx
    push %ecx
    push %ebx
    push %eax
.endm

.macro popa
    pop %eax
    pop %ebx
    pop %ecx
    pop %edx
    pop %esp
    pop %ebp
    pop %esi
    pop %edi
.endm

.macro pushr
	pusha

	push %ds
	push %es
	push %fs
	push %gs 
	push %ebx
	mov $0x10, %bx
	mov %bx, %ds
	pop %ebx
.endm

.macro popr
	mov $0x20, %al
	out %al, $0x20
	
	pop %gs
	pop %fs
	pop %es
	pop %ds

	popa 
.endm

isr_no_error_code 0
isr_no_error_code 1
isr_no_error_code 2
isr_no_error_code 3
isr_no_error_code 4
isr_no_error_code 5
isr_no_error_code 6
isr_no_error_code 7
isr_error_code 8
isr_no_error_code 9
isr_error_code 10
isr_error_code 11
isr_error_code 12
isr_error_code 13
isr_error_code 14
isr_no_error_code 15
isr_no_error_code 16
isr_no_error_code 17
isr_no_error_code 18
isr_no_error_code 19
isr_no_error_code 20
isr_no_error_code 21
isr_no_error_code 22
isr_no_error_code 23
isr_no_error_code 24
isr_no_error_code 25
isr_no_error_code 26
isr_no_error_code 27
isr_no_error_code 28
isr_no_error_code 29
isr_no_error_code 30
isr_no_error_code 31
irq 0
irq 1
irq 2
irq 3
irq 4
irq 5
irq 6
irq 7
irq 8
irq 9
irq 10
irq 11
irq 12
irq 13
irq 14
irq 15

.global asm_syscall
asm_syscall:
    cli
    pushr
    push %eax
    call syscall
    add $4, %esp
    popr
    sti
    iret