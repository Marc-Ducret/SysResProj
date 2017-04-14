.code32 

.section .text
.global _start
_start:
	call main
	jmp _start
