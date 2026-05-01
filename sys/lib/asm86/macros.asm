; ----------------------------------------------------------------------------------------
; FILE: macros.asm
; PURPOSE: Assembly macros used in low level code and debugging
; PROJECT: DragonWare Kernel
; DATE: 12-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

%define BOCHS_BREAK xchg bx, bx
%define MULTIBOOT_MAGIC 0x2BADB002
%macro STOP_EXECUTION 0
	.brk: jmp .brk
%endmacro
