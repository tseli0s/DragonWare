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

%macro SAVE_BASE_POINTER 0
        push    ebp
        mov     ebp, esp
%endmacro

%macro RESTORE_BASE_POINTER 0
        pop ebp
%endmacro

global __make_syscall_ia32_0param
global __make_syscall_ia32_1param
global __make_syscall_ia32_2param
global __make_syscall_ia32_3param
global __make_syscall_ia32_4param
global __make_syscall_ia32_5param

global __make_syscall_ia32_0param_reti32
global __make_syscall_ia32_1param_reti32
global __make_syscall_ia32_2param_reti32
global __make_syscall_ia32_3param_reti32
global __make_syscall_ia32_4param_reti32
global __make_syscall_ia32_5param_reti32

__make_syscall_ia32_0param:
        SAVE_BASE_POINTER

        mov     eax, [ebp+8]
        DRAGONWARE_NATIVE_SYSCALL

        RESTORE_BASE_POINTER
        ret


__make_syscall_ia32_1param:
        SAVE_BASE_POINTER
        push    ebx

        mov     eax, [ebp+8]
        mov     ebx, [ebp+12]
        DRAGONWARE_NATIVE_SYSCALL

        pop ebx

        RESTORE_BASE_POINTER
        ret


__make_syscall_ia32_2param:
        SAVE_BASE_POINTER
        push    ebx

        mov     eax, [ebp+8]
        mov     ebx, [ebp+12]
        mov     ecx, [ebp+16]
        DRAGONWARE_NATIVE_SYSCALL

        pop ebx

        RESTORE_BASE_POINTER
        ret


__make_syscall_ia32_3param:
        SAVE_BASE_POINTER
        push    ebx

        mov     eax, [ebp+8]
        mov     ebx, [ebp+12]
        mov     ecx, [ebp+16]
        mov     edx, [ebp+20]
        DRAGONWARE_NATIVE_SYSCALL

        pop ebx

        RESTORE_BASE_POINTER
        ret


__make_syscall_ia32_4param:
        SAVE_BASE_POINTER
        push    ebx
        push    esi

        mov     eax, [ebp+8]
        mov     ebx, [ebp+12]
        mov     ecx, [ebp+16]
        mov     edx, [ebp+20]
        mov     esi, [ebp+24]
        DRAGONWARE_NATIVE_SYSCALL

        pop esi
        pop ebx

        RESTORE_BASE_POINTER
        ret


__make_syscall_ia32_5param:
        SAVE_BASE_POINTER
        push    ebx
        push    esi
        push    edi

        mov     eax, [ebp+8]
        mov     ebx, [ebp+12]
        mov     ecx, [ebp+16]
        mov     edx, [ebp+20]
        mov     esi, [ebp+24]
        mov     edi, [ebp+28]
        DRAGONWARE_NATIVE_SYSCALL

        pop edi
        pop esi
        pop ebx

        RESTORE_BASE_POINTER
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

__make_syscall_ia32_5param_reti32:
        jmp __make_syscall_ia32_5param
