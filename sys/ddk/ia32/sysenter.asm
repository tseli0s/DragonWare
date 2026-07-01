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
USER_DS_SELECTOR        equ     0x23    ; see in _SysenterEntry

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
; We construct it upon entry and then call the system call handler to handle the actual userland system call.
_SysenterEntry:
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

        ; So, long story short. I hit a bug while developing this feature that had me completely puzzled for months.
        ; Today it's 1st of July 2026, I opened the sysenter support pull request on May 10th.
        ; Now on the bug: If I pressed keys way too fast, or a lot of scrolling was done by the console, suddenly you had a
        ; general protection fault in a move instruction. The fuck????
        ; Well, after tons of debugging with gdb, and with the article on osdev.org not being clear enough for this case, I found out
        ; that the segment selectors were set to 0. Obviously invalid, but I don't touch them anywhere, so even more "what the fuck?".
        ; Oh, even worse, QEMU didn't reproduce this bug at all. Only Bochs did.
        ;
        ; As it turns out, x86 CPUs automatically zero out any of DS/ES/FS/GS registers whose DPL is more privileged than the
        ; new CPL during the transition to userspace. And even worse, this would only manifest in a very specific, rare scenario where
        ; the servers would voluntarily yielded causing the scheduler to switch to another thread causing that security protection to kick in
        ; A DS set to zero is invalid, hence the crash with a #GP.
        ;
        ; (Source: Intel developer manuals, volume 3, chapter 6, and https://github.com/torvalds/linux/blob/master/arch/x86/entry/entry_32.S,
        ; though the latter is far more complicated because it caters to a different internla design)
        mov     ax,     USER_DS_SELECTOR
        mov     ds,     ax
        mov     es,     ax
        mov     fs,     ax
        mov     gs,     ax

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
