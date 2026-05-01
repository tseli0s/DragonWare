; ----------------------------------------------------------------------------------------
; FILE: task.asm
; PURPOSE: Thread-related assembly stubs
; PROJECT: DragonWare Kernel
; DATE: 02-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

bits 32

USER_DATA_SEL   equ     0x23

section .text

global _ThreadStalled
global _ThreadStartup

; void noreturn _ThreadStalled(void)
; To be used when a thread returns, to keep the processor in a low power state
_ThreadStalled:
        hlt                     ; Try to keep the processor in a low power state
        jmp _ThreadStalled      ; Do this infinitely so that the thread never actually returns

_ThreadStartup:
        mov     eax,    [esp+4] ; Load the trap frame
        mov     esp,    eax

        ; Now prepare the segment selectors for the user process
        mov     ax,     USER_DATA_SEL
        mov     ds,     ax
        mov     es,     ax
        mov     fs,     ax
        mov     gs,     ax

        ; And now we can jump into userspace with this thread
        iret
