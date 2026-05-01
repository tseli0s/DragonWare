/**********************************************************************
 * FILE: interrupts.c
 * PURPOSE: Generic interrupts helpers and InterruptStackFrame definition
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/* Must always be inline otherwise the compiler complains at -O3 */
[[gnu::always_inline]]
static inline Bool InterruptsEnabled(void) {
        unsigned long flags;
        __asm__ volatile(
                "pushf\n\t"
                "pop %0"
                : "=r"(flags));
        return flags & (1 << 9);
}

typedef struct _InterruptStackFrame {
        u32 gs, fs, es, ds;
        u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
        u32 int_no;
        u32 err_code;
        u32 eip, cs, eflags;
        u32 useresp, ss;
} InterruptStackFrame;

[[gnu::always_inline]]
static inline void DisableInterrupts(void) {
        __asm__ volatile("cli");
}

[[gnu::always_inline]]
static inline void EnableInterrupts(void) {
        __asm__ volatile("sti");
}
