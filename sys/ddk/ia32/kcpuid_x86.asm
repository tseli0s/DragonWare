; ----------------------------------------------------------------------------------------
; FILE: kcpuid_x86.asm
; PURPOSE: CPUID x86 instruction helpers and low level routines
; PROJECT: DragonWare Freestanding Library
; DATE: 12-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

global __check_cpuid_exists

; int __check_cpuid_exists(void)
; Taken from https://wiki.osdev.org/CPUID

__check_cpuid_exists:
        pushfd                               ;Save EFLAGS
        pushfd                               ;Store EFLAGS

        xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
        popfd                                ;Load stored EFLAGS (with ID bit inverted)
        pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
        
        pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
        xor eax,[esp]                        ;eax = whichever bits were changed
        popfd                                ;Restore original EFLAGS
        and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
        ret
