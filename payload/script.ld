
SECTIONS
{
	.text (0x1000) : {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}

	.data : {
		*(.data)
	}
}
