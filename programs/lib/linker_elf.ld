ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")

SECTIONS
{
	. = 0x40000000;

	.text ALIGN(0x1000) :
	{
	    start.o(.text)
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
}
