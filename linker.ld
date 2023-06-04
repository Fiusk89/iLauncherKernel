ENTRY(start);

SECTIONS {
    . = 0x00100000;

    .multiboot.data ALIGN(0x1000) :
	{
        *(.multiboot.data)
    }

    .multiboot.text ALIGN(0x1000) :
	{
        *(.multiboot.text)
    }

	. += 0xC0000000;

	.text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.text)
	}

	.rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	.data ALIGN(0x1000) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}

	.bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}

    kernel_end = ALIGN(0x1000);
}