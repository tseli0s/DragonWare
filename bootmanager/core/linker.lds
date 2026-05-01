/* ELF loading is complicated and we can't do it in 512 bytes. 
 * We have to output a flat binary here. */
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(BootloaderMain)

SECTIONS {
        . = 0x7C00;
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
}
