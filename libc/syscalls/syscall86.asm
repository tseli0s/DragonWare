; ----------------------------------------------------------------------------------------
; FILE: syscall86.asm
; PURPOSE: System call routines using software interrupts (TODO: Fast syscall track)
; PROJECT: DragonWare C Library
; DATE: 01-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

%define DRAGONWARE_NATIVE_SYSCALL       int 0x60
%define DRAGONWARE_UNI_SYSCALL          int 0x80

global __make_syscall_ia32_0param
global __make_syscall_ia32_1param
global __make_syscall_ia32_2param
global __make_syscall_ia32_3param
global __make_syscall_ia32_4param

global __make_syscall_ia32_0param_reti32
global __make_syscall_ia32_1param_reti32
global __make_syscall_ia32_2param_reti32
global __make_syscall_ia32_3param_reti32
global __make_syscall_ia32_4param_reti32
global __make_syscall_ia32_5param_reti32

__make_syscall_ia32_0param:
        mov     eax,    [esp+4]
        DRAGONWARE_NATIVE_SYSCALL
        ret


__make_syscall_ia32_1param:
        push    ebx
        mov     eax,    [esp+8]
        mov     ebx,    [esp+12]

        DRAGONWARE_NATIVE_SYSCALL

        pop	ebx
        ret


__make_syscall_ia32_2param:
        push    ebx
	push	esi

        mov     eax, [esp+12]
        mov     ebx, [esp+16]
        mov     esi, [esp+20]
        DRAGONWARE_NATIVE_SYSCALL

	pop	esi
        pop	ebx
        ret

__make_syscall_ia32_3param:
        push    ebx
	push	esi
	push	edi

        mov     eax, [esp+16]
        mov     ebx, [esp+20]
        mov     esi, [esp+24]
        mov     edi, [esp+28]
        DRAGONWARE_NATIVE_SYSCALL

	pop	edi
	pop	esi
        pop	ebx
        ret


__make_syscall_ia32_4param:
        push    ebx
        push    esi
	push	edi
	push	ebp

        mov     eax, [esp+20]
        mov     ebx, [esp+24]
        mov     esi, [esp+28]
        mov     edi, [esp+32]
        mov     ebp, [esp+36]
        DRAGONWARE_NATIVE_SYSCALL

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
        ret


; Since the argument is returned in eax by the kernel, these are the same routines
; more or less, so a simple jump will suffice.
__make_syscall_ia32_0param_reti32:
        jmp __make_syscall_ia32_0param

__make_syscall_ia32_1param_reti32:
        jmp __make_syscall_ia32_1param

__make_syscall_ia32_2param_reti32:
        jmp __make_syscall_ia32_2param

__make_syscall_ia32_3param_reti32:
        jmp __make_syscall_ia32_3param

__make_syscall_ia32_4param_reti32:
        jmp __make_syscall_ia32_4param
