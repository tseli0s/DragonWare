; ----------------------------------------------------------------------------------------
; FILE: gdt_x86.asm
; PURPOSE: GDT installation and helper routines in low level assembly
; PROJECT: DragonWare Kernel
; DATE: 09-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------
 
global FlushGDT

; void FlushGDT(u32 gdtptr)
; Takes a GDTPointer (see gdt.h), loads it using lgdt,
; performs a far jump into the new kernel code segments,
; then reloads the segment registers with their appropriate kernel
; values for data and returns.
;
; GPRs are preserved.
FlushGDT:
	mov eax, [esp+4]
	pushad
	lgdt [eax]
	jmp 0x08:.NewCS
.NewCS:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	
	popad
	ret
