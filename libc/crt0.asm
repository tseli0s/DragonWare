; ----------------------------------------------------------------------------------------
; FILE: crt0.asm
; PURPOSE: Startup routine (entry point) for every C program, the commonly seen function _start
; PROJECT: DragonWare C Library
; DATE: 03-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------
bits    32

section .entry
extern  main
global  _start

SYSCALL_EXIT    equ     0x1
DW_SYSCALL_NO   equ     0x60

_start:
        ; Common function prologue for backtraces
        push    ebp
        mov     ebp,    esp

        ; Before we call this, we should probably initialize libc's internal stuff
        ; like the allocator and other things. But since we don't have any of that yet,
        ; this comment is simply a placeholder for the future.
        push    ecx     ; argv (Passed in ecx. Actually nothing passed at all for now the kernel doesn't support it)
        push    eax     ; argc (arguments go right to left because this is a stack, LIFO structure)
        
        call    main
        jmp     __exit_syscall86_internal

; Avoid an extra call inside libc, by defining and performing the system call manually.
; The return code must be put in ecx first, before jumping here. Although, in practice, the
; code isn't used at all - The _DWExit system call takes no arguments.
__exit_syscall86_internal:
        mov     eax,    SYSCALL_EXIT
        int     DW_SYSCALL_NO
