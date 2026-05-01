; ----------------------------------------------------------------------------------------
; FILE: mmutils.asm
; PURPOSE: Handwritten Assembly implementations for memory functions
; PROJECT: DragonWare Freestanding Library
; DATE: 11-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

global memcpy
global memset
global memcmp
global kzeromem

; void *memcpy(void *dest, const void *src, Size n)
; TODO: Use vectored operations to speed this up (this will help a TON...)
memcpy:
        push    edi
        push    esi

        mov     edi,    [esp+12]
        mov     esi,    [esp+16]
        mov     ecx,    [esp+20]
        mov     eax,    edi

        mov     edx,    ecx
        shr     ecx,    2
        cld
        rep     movsd

        mov     ecx,    edx
        and     ecx,    3
        rep     movsb

        pop     esi
        pop     edi
        ret

; void kzeromem(void *dest, Size size) 
kzeromem:
	push    edi

        mov     edi, [esp+8]
        mov     ecx, [esp+12]

        ; If size is 0, return immediately to avoid alignment bugs
        test    ecx,    ecx
        jz      .DoneZeroing

        xor     eax,    eax
        cld

        test    edi,    3
        jz      .AlignedCopy
.AlignData:
        stosb
        dec     ecx
        jz      .DoneZeroing
        test    edi, 3
        jnz     .AlignData
.AlignedCopy:
        mov     edx,    ecx
        shr     ecx,    2
        rep     stosd

        mov     ecx,    edx
        and     ecx,    3
        rep     stosb
.DoneZeroing:
	pop     edi
        ret

; void *memset(void *obj, int value, Size size)
memset:
        push    edi
        mov     edi,    [esp+8]
        mov     al,     [esp+12]
        mov     ecx,    [esp+16]

        ; Copy the byte in AL to all 4 bytes of EAX, so that movsd
        ; can work. So if al is 0xAB, EAX will become 0xABABABAB
        ; No that wasn't my idea I'm not that genius, but apparently it works
        imul    eax,    0x01010101
        
        mov     edx,    ecx
        shr     ecx,    2
        cld
        rep     stosd
        
        mov     ecx,    edx
        and     ecx,    3
        rep     stosb

        mov     eax,    [esp+8]
        pop     edi
        ret

; int memcmp(const void *p1, const void *p2, Size n)
memcmp:
        push    edi
        push    esi
        push    ebx

        mov     edi,    [esp+16]
        mov     esi,    [esp+20]
        mov     ecx,    [esp+24]

        xor     eax,    eax 
        test    ecx,    ecx
        jz      .Done

        mov     edx,    ecx
        shr     ecx,    2
        jz      .HasResidualBytes

        cld
        repe    cmpsd
        je      .HasResidualBytes

        ; There's a difference in the last dword we compared - Back up
        ; and check byte by byte to find the exact point
        sub     edi,    4
        sub     esi,    4
        mov     ecx,    4
        jmp     .ByteCheck

.HasResidualBytes:
        mov     ecx,    edx
        and     ecx,    3
        jz      .Done

.ByteCheck:
        repe    cmpsb
        je      .Done

.Diff:
        movzx   eax,    byte[edi-1]
        movzx   ebx,    byte[esi-1]
        sub     eax,    ebx

.Done:
        pop     ebx
        pop     esi
        pop     edi
        ret
