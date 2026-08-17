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

        ; Besides the iret frame, we have also pushed the function argument in the kernel stack,
        ; and it sits right below the iret frame.
        mov     ebx,    [esp+12] ; useresp
        mov     ecx,    [esp+20] ; extra_data (main function argument)

        ; Because we haven't switched to the user stack yet (iret will do this for us),
        ; we have to manually push the extra_data argument on the stack, along with a dummy
        ; return address to pad it off.
        sub     ebx,    4
        mov     dword   [ebx],  ecx

        ; This is just some technicality, the compiler expects the first argument at [esp+4]
        ; so [esp] must technically point to the return address. If a thread 
        sub     ebx,    4
        mov     dword   [ebx],  0x00000000

        ; Now overwrite the old useresp address with the new address (which is lower by eight bytes,
        ; as we've pushed the function argument in there and a dummy return address).
        mov     [esp+12],       ebx

        ; Some register cleanup just to be on the safe side.
        xor     dword   ebx,    ebx
        xor     dword   ecx,    ecx
        xor     dword   edx,    edx
        xor     dword   ebp,    ebp
        xor     dword   esi,    esi
        xor     dword   edi,    edi

        ; Now prepare the segment selectors for the user process
        mov     ax,     USER_DATA_SEL
        mov     ds,     ax
        mov     es,     ax
        mov     fs,     ax
        mov     gs,     ax

        ; More cleanup just to be sure :P
        xor     eax,    eax

        ; And now we can jump into userspace with this thread
        iret
