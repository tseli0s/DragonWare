; ----------------------------------------------------------------------------------------
; FILE: string.asm
; PURPOSE: String and memory manipulation standard functions implemented in x86 assembly
; PROJECT: DragonWare C Library
; DATE: 06-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

section .text
global  memcpy
global  memset
global  strcpy
global  strlen
global  memcmp

memcpy:
        push    ebp
        mov     ebp,    esp

        push    esi
        push    edi

        mov     ecx,    [ebp+16]
        test    ecx,    ecx
        jz      .done

        mov     edi,    [ebp+8]
        mov     esi,    [ebp+12]
        mov     eax,    edi

        mov     edx,    ecx
        shr     ecx,    2
        rep     movsd

        mov     ecx,    edx
        and     ecx,    3
        rep     movsb
.done:
        mov     eax,    [ebp+8]
        pop     edi
        pop     esi
        pop     ebp
        ret

memset:
        push    ebp
        mov     ebp,    esp

        push    edi
        mov     edi,            [ebp+8]
        movzx   eax,    byte    [ebp+12]
        mov     ecx,            [ebp+16]

        ; Copy the byte in AL to all 4 bytes of EAX, so that movsd
        ; can work. So if al is 0xAB, EAX will become 0xABABABAB
        ; No that wasn't my idea I'm not that genius, but apparently it works
        imul    eax,    0x01010101
        
        mov     edx,    ecx
        shr     ecx,    2
        rep     stosd
        
        mov     ecx,    edx
        and     ecx,    3
        rep     stosb

        mov     eax,    [ebp+8]
        pop     edi
        pop     ebp
        ret

memcmp:
        push    ebp
        mov     ebp,    esp
        push    esi
        push    edi

        mov     esi,    [ebp+8]
        mov     edi,    [ebp+12]
        mov     ecx,    [ebp+16]

        test    ecx,    ecx
        jz      .match

        repe    cmpsb
        je      .match

        movzx   eax,    byte [esi-1]
        movzx   edx,    byte [edi-1]
        sub     eax,    edx
        jmp     .done
.match:
        xor     eax,    eax
.done:
        pop     edi
        pop     esi
        pop     ebp
        ret

strcpy:
        push    ebp
        mov     ebp,    esp

        push    edi
        push    esi

        mov     edi,    [ebp+8]
        mov     esi,    [ebp+12]

.loop:
        mov     al,     [esi]
        mov     [edi],  al

        inc     esi
        inc     edi

        test    al,     al
        jnz     .loop
.done:
        mov     eax,    [ebp+8]
        pop     esi
        pop     edi
        pop     ebp
        ret

strlen:
        push    ebp
        mov     ebp,    esp
        mov     eax,    [ebp+8]
.len_loop:
        cmp     byte    [eax],  0
        je      .done
        inc     eax
        jmp     .len_loop
.done:
        sub     eax,    [ebp+8]
        pop     ebp
        ret
