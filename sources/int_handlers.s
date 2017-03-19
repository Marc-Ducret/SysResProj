.macro isr_error_code nb
.global isr\nb
isr\nb:
    cli
    push $\nb
    jmp isr_common_stub
.endm

.macro isr_no_error_code nb
.global isr\nb
isr\nb:
    cli
    push $0
    push $\nb
    jmp isr_common_stub
.endm

.macro pusha
    push %edi
    push %esi
    push %ebp
    push %esp
    push %ebx
    push %edx
    push %ecx
    push %eax
.endm

.macro popa
    pop %eax
    pop %ecx
    pop %edx
    pop %ebx
    pop %esp
    pop %ebp
    pop %esi
    pop %edi
.endm

isr_common_stub:
   pusha                    # Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

   movw %ds, %ax              # Lower 16-bits of eax = ds.
   push %eax                 # save the data segment descriptor

   movw $0x10, %ax  # load the kernel data segment descriptor
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs

   call isr_handler

   pop %eax        # reload the original data segment descriptor
   movw %ax, %ds
   movw %ax, %es
   movw %ax, %fs
   movw %ax, %gs

   popa                     # Pops edi,esi,ebp...
   add $8, %esp     # Cleans up the pushed error code and pushed ISR number
   sti
   iret

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