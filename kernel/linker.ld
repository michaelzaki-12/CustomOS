OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

SECTIONS
{
    . = 32M;
	.text : ALIGN(0x1000)
	{
		*(.text)
	}
	.rodata : ALIGN(0x1000)
	{
		*(.rodata)
	}
	.data : ALIGN(0x1000)
	{
		*(.data)
	}
	.bss : ALIGN(0x1000)
	{
		*(COMMON)
		*(.bss)
	}
}