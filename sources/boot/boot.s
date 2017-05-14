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

.code32 

# Allocate room for a small temporary stack as a global variable called stack.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded.
.section .text
.global _start
_start:
	# First, we'll set the stack pointer to the top of our stack declared above.
	movl $stack_top, %esp
	
	# Now that we have a stack, we can provide the minimal environment needed to
	# run C code.

	# Now that the initial bootstrap environment is set up, call the kernel's
	# main function using the C calling convention.
	push %ebx
	call kmain
	cli
	hlt
