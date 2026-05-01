/**********************************************************************
 * FILE: irq.h
 * PURPOSE: IRQ implementation based on DragonWare's kernel
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define MAX_IRQ    (16)
#define IRQ_ACCESS (0x30)

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _IRQRegisters {
        u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
        u32 int_no;
        u32 eip, cs, eflags;
} IRQRegisters;

typedef void (*IRQHandler)(IRQRegisters *);

void RegisterIRQHandler(int which, IRQHandler handler);
void IRQInit(void);
