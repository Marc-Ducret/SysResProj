/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	. = 0x1500; /* ?? wtf */

	.text BLOCK(4K) : ALIGN(4K)
	{
		//. = 0x1500;
		boot.o(.ninja);
		*(.text);
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata);
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data);
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON);
		*(.bss);
		*(.bootstrap_stack);
	}

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}
