; ----------------------------------------------------------------------------------------
; FILE: multiboot.asm
; PURPOSE: Multiboot1 bootstrap helpers and headers, used in conjunction with bootia32.asm
; PROJECT: DragonWare Kernel
; DATE: 02-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

[bits 32]

extern _ebss
extern _SystemBootstrapRoutine

%define MULTIBOOT_DEFAULT_WIDTH         0
%define MULTIBOOT_DEFAULT_HEIGHT        0
%define MULTIBOOT_FRAMEBUFFER_MODE      0
%define DEFAULT_BITS_PER_PIXEL          32

MB_ALIGN                equ        1<<0
MB_MEMINFO              equ        1<<1
MB_VIDEO_MODE           equ        1<<2
MB_FLAGS        	equ        MB_ALIGN | MB_MEMINFO | MB_VIDEO_MODE
MB_MAGIC        	equ        0x1BADB002
MB_CHECKSUM     	equ        -(MB_MAGIC + MB_FLAGS)

section .multiboot
align 4
dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM

; Load address fields
; We only inform the bootloader of .bss and _start,
; everything else is handled automatically

dd 0x0                          ; Multiboot header address, the bootloader probes it on its own
dd 0x0                          ; Beginning of the .text segment, not used yet
dd 0x0                          ; End of the .data segment. Broken unless we use the a.out format.
dd _ebss                        ; End of the .bss segment
dd _SystemBootstrapRoutine      ; Where should the bootloader jump at entry. Already configured in the linker script but let's be explicit.

; Video mode fields
; This is where we decide between text mode and a proper framebuffer, we
; let the bootloader pick the resolutions and everything else.
dd MULTIBOOT_FRAMEBUFFER_MODE
dd MULTIBOOT_DEFAULT_WIDTH
dd MULTIBOOT_DEFAULT_HEIGHT
dd DEFAULT_BITS_PER_PIXEL
