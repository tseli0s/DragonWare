; ----------------------------------------------------------------------------------------
; FILE: vmm_x86.asm
; PURPOSE: Low level routines to access virtual memory related functionality on the hardware
; PROJECT: DragonWare Kernel
; DATE: 11-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

global SetPageDirectory
global FlushTLB

; void SetPageDirectory(uintptr_t addr);
; addr must be a physical address. Thanks to the C [[gnu::regparm(1)]] attribute,
; the parameter is directly in eax when this function is called to drop an extra memory access.
SetPageDirectory:
        mov     cr3, eax
        ret

; void FlushTLB(void)
; quick and dirty function to force the CPU to reload
; the paging structures (and therefore flush the TLB)
FlushTLB:
        mov     eax, cr3
        mov     cr3, eax
        ret
