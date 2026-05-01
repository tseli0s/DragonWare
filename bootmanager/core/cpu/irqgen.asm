; ----------------------------------------------------------------------------------------
; FILE: irqgen.asm
; PURPOSE: IRQ stub autogeneration using assembler macros
; PROJECT: DragonWare Boot Manager
; DATE: 08-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

extern IRQHandlerCallback
%macro IRQ_STUB 1
global irq%1
irq%1:
	cli
	push dword %1 + 32
	pushad

	mov eax, esp
	push eax
	call IRQHandlerCallback
	add esp, 4

	popad
	add esp, 4

	%if %1 >= 8
		mov al, 0x20
		out 0xA0, al
	%endif
	mov al, 0x20
	out 0x20, al
	iret
%endmacro

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15
