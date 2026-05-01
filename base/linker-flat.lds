/*
 * This is a linker script only used for flat binaries. 
 * DragonWare doesn't support ELF files (For now, this is obviously going to change), so
 * all files are meant to be flat binaries that the kernel simply copies into a separate
 * virtual address space.
 */

OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS {
        /* This is where the entry point is configured to be by default. */
        . = 0x1000;

        .text : ALIGN(4) {
                *(.entry*)
                *(.text*)
        }

        .rodata : ALIGN(4) {
                *(.rodata*)
        }

        .data : ALIGN(4) {
                *(.data*)
        }

        .bss : ALIGN(4) {
                __bss_start = .;
                *(.bss*)
                *(COMMON)
                __bss_end = .;
        }
        /DISCARD/ : {
			*(.note .note.*)
			*(.eh_frame*)
			*(.comment)
	}
        . = ALIGN(4);
}
