; ----------------------------------------------------------------------------------------
; FILE: syscall86.asm
; PURPOSE: System call routines using software interrupts (TODO: Fast syscall track)
; PROJECT: DragonWare C Library
; DATE: 01-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

section .data
; By default, good old software interrupts will be used for system calls
__syscall_gate_internal:
        dd     __syscall_trap_int0x60

section .text
global __libc_try_enable_sysenter
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

; Checks if the machine supports sysenter. Returns 0 if it is supported,
; 1 if it isn't. Assumes the host computer supports the cpuid instruction.
; TODO: Obviously, if the host machine doesn't support CPUID, we should check that,
; or put the check behind _DW_LEGACY_SUPPORT that is specifically for pre-Pentium II CPUs.
; Also, AMD chips often don't play well with sysenter and sysexit. There should be a check for that too
; in here.
__libc_check_sysenter:
        push    ebx
        mov     eax,    0x01
        cpuid
        test    edx,    0x800   ; Bit 11 (SEP)
        pop     ebx
        jz      .not_supported
        xor     eax,    eax
        ret
.not_supported:
        mov     eax,    0x01
        ret

__syscall_trap_int0x60:
        int     0x60
        ret

__syscall_trap_sysenter:
        mov     ecx,    esp
        mov     edx,    .ReturnFromSysenter
        sysenter
.ReturnFromSysenter:
        ret

__libc_try_enable_sysenter:
        call    __libc_check_sysenter
        test    eax,    eax
        jz .EnableSysenter
        mov dword       [__syscall_gate_internal], __syscall_trap_int0x60
        ret
.EnableSysenter:
        mov dword       [__syscall_gate_internal], __syscall_trap_sysenter
        ret

__make_syscall_ia32_0param:
        mov     eax,    [esp+4]
        call    [__syscall_gate_internal]
        ret

__make_syscall_ia32_1param:
        push    ebx
        mov     eax,    [esp+8]
        mov     ebx,    [esp+12]

        call    [__syscall_gate_internal]
        pop     ebx
        ret


__make_syscall_ia32_2param:
        push    ebx
	push	esi

        mov     eax, [esp+12]
        mov     ebx, [esp+16]
        mov     esi, [esp+20]
        call    [__syscall_gate_internal]

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
        call    [__syscall_gate_internal]

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
        call    [__syscall_gate_internal]

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
