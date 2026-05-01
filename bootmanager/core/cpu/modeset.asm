; ----------------------------------------------------------------------------------------
; FILE: modeset.asm
; PURPOSE: BIOS-based framebuffer modesetting
; PROJECT: DragonWare 
; DATE: 03-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------
bits 32
global ModesetInRealMode

SavedESP32: dd 0
SavedIDT:
        dd 0
        dw 0

RealModeIDT:
        dw 0x03FF       ; Limit: 256 interrupts * 4 bytes - 1
        dd 0x00000000   ; Base: IVT is at physical address 0

VideoModeToUse: dd 0x0

; void ModesetInRealMode(u32 mode)
; mode is preserved above.
ModesetInRealMode:
        cli
        mov [SavedESP32], esp
        mov eax, [esp+4]
        mov eax, [eax]
        mov [VideoModeToUse], eax
        sidt [SavedIDT]

        mov ax, 0x20  
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax

        jmp 0x18:Protected16Bit

bits 16
Protected16Bit:
        xor eax, eax
        mov cr0, eax
        jmp 0x00:InRealMode

InRealMode:
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov sp, 0x6C00

        lidt [RealModeIDT]
        sti

        ; Now do the actually cool stuff
        mov ax, 0x4F02
        mov bx, [VideoModeToUse]
        int 0x10

        cli
        mov eax, cr0
        or eax, 1
        mov cr0, eax

        jmp 0x08:BackToProtectedMode

bits 32
BackToProtectedMode:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov esp, [SavedESP32]
        cld

        lidt [SavedIDT]
        sti
        ret
