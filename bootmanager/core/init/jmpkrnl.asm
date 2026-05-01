; ----------------------------------------------------------------------------------------
; FILE: jmpkrnl.asm
; PURPOSE: Trampoline to jump into the loaded kernel and begin booting the operating system.
; PROJECT: DragonWare Boot Manager
; DATE: 02-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

; The routine to jump to the kernel after its loaded and ready to be executed
; NOTES:
; - As per multiboot spec, interrupts and paging are disabled, as well as vm86 mode
; - Everything else can be left as is, the GDT is already initialized when the second stage is loaded as
;   expected by the protocol
; - Parameters: Multiboot information structure (1), jump address (2)

MULTIBOOT_MAGIC_BOOT equ 0x2BADB002

global _JumpToKernel
; void halting _JumpToKernel(void *multiboot_addr, void *jmpaddr)
; Jumps to the newly loaded kernel
_JumpToKernel:
        cli

        mov eax, MULTIBOOT_MAGIC_BOOT
        mov ebx, [esp+4]
        mov ecx, [esp+8]

        jmp ecx
        jmp $   ; Should never be reached
