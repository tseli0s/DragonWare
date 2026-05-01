; ----------------------------------------------------------------------------------------
; FILE: ctxswitch.asm
; PURPOSE: Context switching function for the IA32 architecture
; PROJECT: DragonWare Kernel
; DATE: 03-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

bits 32

section .text
global SwapThreadStack

; void SwapThreadStack(u32 *oldesp, u32 newesp)
SwapThreadStack:
        push    ebp
        push    ebx
        push    esi
        push    edi

        mov     eax, [esp + 20]
        mov     edx, [esp + 24]

        mov     [eax], esp
        mov     esp, edx            

        pop     edi
        pop     esi
        pop     ebx
        pop     ebp

        ret
