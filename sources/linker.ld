/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	. = 0x1100;

	.text ALIGN(0x1000) :
	{
		*(.multiboot);
	    *(.text);
	}

	.rodata ALIGN(0x1000) :
	{
		*(.rodata);
	}

	.data ALIGN(0x1000) :
	{
		*(.data);
	}

	.bss ALIGN(0x1000) :
	{
		*(COMMON);
		*(.bss);
		*(.bootstrap_stack);
	}
	end = .;
}
