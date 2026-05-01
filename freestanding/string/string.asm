; ----------------------------------------------------------------------------------------
; FILE: string.asm
; PURPOSE: Handwritten Assembly implementations for string functions
; PROJECT: DragonWare Freestanding Library
; DATE: 11-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

section .text
global strlen
global strcmp

; Size strlen(const char *s)
strlen:
        mov eax, [esp+4]
.len_loop:
        cmp byte [eax], 0
        je .done_strlen
        inc eax
        jmp .len_loop
.done_strlen:
        sub eax, [esp+4]
        ret
 
; int strcmp(const char *str1, const char *str2)
strcmp:
        mov eax, [esp+4]
        mov ecx, [esp+8]
.loop:
        mov dl,  [eax]
        mov dh,  [ecx]

        cmp dl, dh
        jne .diff

        test dl, dl
        je .equal

        inc eax
        inc ecx
        jmp .loop

.diff:
        movzx eax, dl
        movzx ecx, dh
        sub eax, ecx
        ret
.equal:
        xor eax, eax
        ret
