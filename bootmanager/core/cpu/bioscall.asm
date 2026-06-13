; ----------------------------------------------------------------------------------------
; FILE: bioscall.asm
; PURPOSE: BIOS firmware call helper implementation for x86 PCs 
; PROJECT: DragonWare Boot Manager
; DATE: 06-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

; The code here is taken largely from GNU GRUB, specifically the file
; grub-core/kern/i386/int.S
; Main modifications are translating it to NASM syntax and adapting it to this bootloader's
; structures.

REAL_MODE_STACK                 equ     0x6C00
CODE_SEG_PROTECTED_16           equ     0x18
DATA_SEG_PROTECTED_16           equ     0x20
CODE_SEG_PROTECTED_32           equ     0x08
DATA_SEG_PROTECTED_32           equ     0x10
section .data
ProtectedModeIDT:
        dw      0x0000
        dd      0x00000000
RealModeIDT:
        dw      0x03FF
        dd      0x00000000
ProtectedModeStack:
        dd      0x00000000
ReturnAddress:
        dd      0x00000000

section .text
; Interrupts must be disabled when this function is called.
; This is only called in this file and nowhere else.
ProtectedToRealMode:
        sidt    [ProtectedModeIDT]
        lidt    [RealModeIDT]
        mov     [ProtectedModeStack],   esp

        ; This only works because the bootloader runs in the lower 64KBs of memory
        mov     ax,     [esp]
        mov     word    [REAL_MODE_STACK],      ax

        mov     esp,    REAL_MODE_STACK
        mov     ebp,    esp

        mov     ax,     DATA_SEG_PROTECTED_16
        mov     ds,     ax
        mov     es,     ax
        mov     fs,     ax
        mov     gs,     ax
        mov     ss,     ax

        jmp     CODE_SEG_PROTECTED_16:.__trampoline_16
; Now we are in protected 16 bit mode (yes, that's a thing).
; The only thing we are going to do is simply clear the protected mode bit and go to real mode.
bits    16
.__trampoline_16:
        mov     eax,    cr0
        and     eax,    0xFFFFFFFE      ; Clear bit 0 which is the PE bit (We don't use paging in the bootloader, the rest can stay)
        mov     cr0,    eax

        jmp     0x0000:.RealMode
; Now we are in real mode. Set up the segments (ds, es, ss) and fallthrough to
; the caller's real mode code. You'll get it, hold on.
.RealMode:
        xor     ax,     ax
        mov     ds,     ax
        mov     es,     ax
        mov     fs,     ax
        mov     gs,     ax
        mov     ss,     ax

        ; This is the coolest part. Now that we have set up the registers and everything, we are returning back to the caller,
        ; except the caller is now running in a CPU in real mode. So all code will now run as 16 bit code!
        ret

bits    32
; __do_bios_call
; Called by BIOSCall to perform the actual mode switch to real 16 bit mode, perform the interrupt and return
; Interrupt number goes in %al, and %edx must contain a pointer to a BIOSRegisters structure to load before performing
; the interrupt
;
; BIOSRegisters looks like this:
; typedef struct [[gnu::packed]] _BIOSRegisters {
;       u32 eax, ebx, ecx, edx, esi, edi;
;       u32 es, ds;
; } BIOSRegisters;
__do_bios_call:
        cli

        ; Self modify the instruction. I took this directly from the Linux kernel, because there's no
	; other easy way to do this.
	; The first byte is the INT instruction, 0xCD. The second byte is the actual interrupt we 
	; want to perform.
        mov     byte    [.InterruptNumber],     al

        pushad
        mov     ax,     [edx+24]
        mov     bx,     [edx+28]
        mov     [.RegisterES],  ax
        mov     [.RegisterDS],  bx

        ; Since we need to use EAX for other things, we will also patch it every call
        ; and then restore it here
        mov     eax,            [edx+0]
        mov     [.RegisterEAX], eax

        mov     ebx,    [edx+4]
        mov     ecx,    [edx+8]
        mov     esi,    [edx+16]
        mov     edi,    [edx+20]
        mov     edx,    [edx+12]

        call ProtectedToRealMode

bits    16
.InRealMode:
        pushf
        cli
        push    ds

        db      0xB8            ; MOV   AX,     IMM16 opcode
.RegisterES:
        dw      0x0000
        mov     es,     ax

        db      0xB8
.RegisterDS:
        dw      0x0000
        mov     ds,     ax

        db      0x66,   0xB8    ; MOV DWORD EAX, IMM32 opcode
.RegisterEAX:
        dd      0x00000000
.Interrupt:             db      0xCD    ; INT instruction
.InterruptNumber:       db      0x00    ; Look at the self modification trick above
.ReturnToProtectedMode:
        pop     ds
        popf
        cli
        lidt    [ProtectedModeIDT]

        mov     eax,    cr0
        or      eax,    0x01
        mov     cr0,    eax

        jmp     CODE_SEG_PROTECTED_32:.BackToProtectedMode

bits    32
; TODO: Have this as a separate function as well we can call, just like ProtectedToRealMode above.
; (It's easier to inline it for now, but okay)
.BackToProtectedMode:
        mov     ax,     DATA_SEG_PROTECTED_32
        mov     ds,     ax
        mov     es,     ax
        mov     gs,     ax
        mov     fs,     ax
        mov     ss,     ax
        mov     esp,    [ProtectedModeStack]
        mov     ebp,    esp
        add     esp,    4       ; Discard the stale return address we pushed when calling RET in real mode. Because the
                                ; real mode stack is different, the RET instruction above didn't actually touch this stack, so
                                ; we must clean up ourselves.
        popad
        sti
        ret

global BIOSCall
; void BIOSCall(int vector, BIOSRegisters *regs)
; Temporarily jumps into real mode, performs a BIOS interrupt and returns.
; Results of the BIOS calls are not returned (FIXME)
;
; BIOSRegisters looks like this:
; typedef struct [[gnu::packed]] _BIOSRegisters {
;       u32 eax, ebx, ecx, edx, esi, edi;
;       u32 es, ds;
; } BIOSRegisters;
BIOSCall:
        push    ebp
        mov     ebp,    esp
        pushf

        mov     eax,    [ebp+8]
        mov     edx,    [ebp+12]

        call    __do_bios_call

        popf
        pop     ebp
        ret
