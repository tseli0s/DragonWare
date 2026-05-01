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

; System calls expect this frame:
; typedef struct _InterruptStackFrame {
;         u32 gs, fs, es, ds;
;         u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
;         u32 int_no;
;         u32 err_code;
;         u32 eip, cs, eflags;
;         u32 useresp, ss;
; } InterruptStackFrame;
; Obviously most of the fields are unused in this frame, so...
; TODO: Use a dedicated SystemCallRegisters for system calls to avoid pushing unnecessary
; state here.
;
; NOTE: Userland doesn't make use of this code yet. It uses the traditional software interrupt method for now.
; Meaning this code here is just a stub for the future, and has not been tested yet.
; In fact, the system call ABI used by DragonWare requires the third and fourth arguments to be passed in ecx/edx,
; which are also used by sysenter/sysexit to find the return address and the user stack. Meaning if you just tried to
; use it anyways, best case it may work for SOME programs, but everything else will be broken.
_SysenterEntry:
        cli

        push    ecx     ; useresp
        push    edx     ; usereip (return address)

        ; Now construct the interrupt frame
        ; Bottom 7 fields are unused, just push junk to prepare the actual state
        times 7 push dword 0

        ; GPRs
        pushad

        ; Segment registers
        push    ds
        push    es
        push    fs
        push    gs

        push    esp                     ; Push the stack as the InterruptStackFrame
        call    DragonWareSyscall       ; Call the system call handler
        add     esp,    4               ; Now discard the argument we pushed

        ; Now clean up the stack to only leave the registers we need to return
        ; from the system call.
        add     esp,    16      ; Segment registers, not used.
        popad                   ; Restore the GPRs now with the system call state
        add     esp,    28      ; Discard stuff that we don't actually use in sysenter/sysexit

        pop     edx             ; Return address to drop back to
        pop     ecx             ; User stack to switch to upon return

        sti                     ; Reenable interrupts...
        sysexit                 ; ...and finally return to userspace!
