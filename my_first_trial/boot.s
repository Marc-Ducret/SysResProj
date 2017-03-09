# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


# Allocate room for a small temporary stack as a global variable called stack.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

#define the call of kmain C function
.section .text
.global _start
_start:
        # Sets the pointer to the beginning of the stack (because it goes down)
	movl $stack_top, %esp
        call kmain
        cli
hang: # assures that the OS will never get out of here
        hlt
        jmp hang
