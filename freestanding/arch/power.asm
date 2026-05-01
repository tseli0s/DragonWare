; ----------------------------------------------------------------------------------------
; FILE: power.asm
; PURPOSE: Generic x86 power management functions
; PROJECT: DragonWare Freestanding Library
; DATE: 02-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

section .text
global ForceReboot
global StallMachine

ForceReboot:
        lidt [0x4124814]
        lgdt [0x1492112]
        sti
        jmp ForceReboot

StallMachine:
        cli
.LoopForever:
        hlt
        jmp .LoopForever

