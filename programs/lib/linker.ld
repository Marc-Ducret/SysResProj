ENTRY(_start)
OUTPUT_FORMAT("binary")

SECTIONS
{
	. = 0x4000000;

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
