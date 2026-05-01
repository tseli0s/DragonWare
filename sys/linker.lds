ENTRY(_SystemBootstrapRoutine)
OUTPUT_FORMAT(elf32-i386)

KERNEL_VMA 	= 0xC0000000;
KERNEL_LMA 	= 0x00100000;
KERNEL_MAX_SIZE = 0x200000;
PAGE_SIZE	= 0x1000;

SECTIONS {
	. = KERNEL_VMA;
	. = ALIGN(PAGE_SIZE);
	_begin = .;

	.text : AT(KERNEL_LMA) {
		__text_section_start = .;
		KEEP(*(.multiboot))
       		*(.text*)
		__text_section_end = .;
   	}

	. = ALIGN(PAGE_SIZE);
	.rodata : AT(ADDR(.rodata) + KERNEL_LMA - KERNEL_VMA) {
		__rodata_section_start = .;
		KEEP(*(.fontdata*))
		*(.rodata*)
		__rodata_section_end = .;
	}

	. = ALIGN(PAGE_SIZE);
   	.data : AT(ADDR(.data) + KERNEL_LMA - KERNEL_VMA) {
       		*(.data*);
   	}

	. = ALIGN(PAGE_SIZE);
	.drivers : AT(ADDR(.drivers) + KERNEL_LMA - KERNEL_VMA) {
		__driver_descriptor_start = .;
		KEEP(*(.drivers*));
		__driver_descriptor_end = .;
	}

	. = ALIGN(PAGE_SIZE);					
   	.bss : AT(ADDR(.bss) + KERNEL_LMA - KERNEL_VMA) {
       		_sbss = .;
       		*(COMMON)
       		*(.bss*)
       		_ebss = .;
   	}

	/DISCARD/ : {
		*(.note .note.*)
		*(.eh_frame*)
		*(.comment*)
	}

	/* The kernel's end address. Originally _ebss was used but that is fragile so
	 * a separate symbol is a better practice. */
	_end = .;
}

ASSERT((_end - _begin) <= KERNEL_MAX_SIZE, "Kernel exceeds 2 megabytes in size, cannot link (Try compiling with size optimizations)");
