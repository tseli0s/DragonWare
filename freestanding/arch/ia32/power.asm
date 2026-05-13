; ----------------------------------------------------------------------------------------
; FILE: power.asm
; PURPOSE: Generic x86 power management functions
; PROJECT: DragonWare Freestanding Library
; DATE: 02-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

PS2_PORT_STATUS		equ	0x64	; Controller status port
PS2_PORT_COMMAND	equ	0x64	; Yes they're the same actually one's for input one's for output
PS2_RESET_CPU_CMD	equ	0xFE	; Sending this byte to the PS/2 controller causes a CPU reset
					; (Apparently it was a very powerful controller so they added all sorts
					; of random shit into it)
section .text
global ForceReboot
global StallMachine

; https://wiki.osdev.org/I8042_PS/2_Controller#CPU_Reset
ForceReboot:
; The input buffer must be empty before we submit the command,
; otherwise the reset fails.
.wait:
	in	al,	PS2_PORT_STATUS
	test	al,	0x02
	jne	.wait

	mov	al,	PS2_RESET_CPU_CMD	
	out	PS2_PORT_COMMAND,	al

        jmp ForceReboot	; Failsafe, if the machine somehow survives the reset.

StallMachine:
        cli
.LoopForever:
        hlt
        jmp .LoopForever

