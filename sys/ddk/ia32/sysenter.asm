; ----------------------------------------------------------------------------------------
; FILE: sysenter.asm
; PURPOSE: sysenter/sysexit support for DragonWare
; PROJECT: DragonWare Kernel
; DATE: 04-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

IA32_SYSENTER_CS        equ     0x174
IA32_SYSENTER_ESP       equ     0x175
IA32_SYSENTER_EIP       equ     0x176
KERNEL_CS_ENTRY         equ     0x08

bits 32
section .text

extern DragonWareSyscall

; void EnableSysenter(void)
global EnableSysenter
EnableSysenter:
        push    ebp
        mov     ebp,    esp

        ; First we must configure the MSRs to contain the data used by sysenter/sysexit
        ; Documentation can be found here: https://wiki.osdev.org/SYSENTER
        mov     ecx,    IA32_SYSENTER_CS
        xor     edx,    edx
        mov     eax,    KERNEL_CS_ENTRY
        wrmsr

        ; BIG FAT ASS WARNING HERE: Yes, the stack is set to 0. The idea being that it will be overwritten
        ; every time each process is switched around. Don't worry, it's being handled in gdt.c
        mov     ecx,    IA32_SYSENTER_ESP
        xor     eax,    eax
        wrmsr

        mov     ecx,    IA32_SYSENTER_EIP
        mov     eax,    _SysenterEntry
        wrmsr
        
        pop     ebp
        ret

;
; System calls expect this frame:
; typedef struct [[gnu::packed]] _SystemCallFrame {
;         u32 ebx, esi, edi, ebp; /* Arguments 0-3 of every system call */
;         u32 eax;                /* System call number */
; } SystemCallFrame;
;
; We construct it upon entry and then call the system call handler to handle the actual userland system call.
; NOTE: Userland doesn't make use of this code yet. It uses the traditional software interrupt method for now.
; Meaning this code here is just a stub for the future, and has not been tested yet.
_SysenterEntry:
        cli             ; Disable interrupts. The kernel doesn't support reentrancy yet.
        push    ecx     ; useresp
        push    edx     ; usereip (return address)

        ; Now we must construct the SystemCallFrame and give it to the
        ; system call handler.
        push    eax
        push    ebp
        push    edi
        push    esi
        push    ebx

        push    esp                     ; Push the stack as the SystemCallFrame
        call    DragonWareSyscall       ; Call the system call handler
        add     esp,    4               ; Now discard the argument we pushed

        ; Get whatever the kernel returned into those arguments and restore it
        ; for the user process. Most importantly, eax holds the return code and
        ; esi/edi may hold extra return values.
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        pop     eax

        pop     edx                     ; Return address to drop back to
        pop     ecx                     ; User stack to switch to upon return
        sti
        sysexit
