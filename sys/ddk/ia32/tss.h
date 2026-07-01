/**********************************************************************
 * FILE: tss.h
 * PURPOSE: Task Segment Selector struct definition and ltr instruction helper.
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "task/process.h"

/* Eight bits per byte, 8192 bytes, thats ~65000 bits, each clear bit
 * is a port that the current process is allowed to use */
#define TSS_IOPERM_SIZE (8192)

/* For future readers: Most of these fields will go unused. We
 * don't do hardware task switching because it's slow and inflexible. */
typedef struct [[gnu::packed]] _TSSEntry {
        u16  prev_tss, reserved0;
        u32  esp0;
        u16  ss0, reserved1;
        u32  esp1;
        u16  ss1, reserved2;
        u32  esp2;
        u16  ss2, reserved3;
        u32  cr3;
        u32  eip;
        u32  eflags;
        u32  eax, ecx, edx, ebx;
        u32  esp, ebp, esi, edi;
        u16  es, reserved4;
        u16  cs, reserved5;
        u16  ss, reserved6;
        u16  ds, reserved7;
        u16  fs, reserved8;
        u16  gs, reserved9;
        u16  ldt_selector, reserved10;
        u16  trap, iomap_base;
        Byte ioperm[TSS_IOPERM_SIZE];
        Byte terminator; /* Must always be 0xFF */

} TSSEntry;

[[gnu::always_inline]]
static inline void __ltr(u16 tptr) {
        __asm__ volatile(
                "movw %0, %%ax\n"
                "ltr %%ax\n" ::"r"(tptr)
                : "ax");
}

/**
 * @brief Enables access to the I/O ports of the process
 * by setting up the relevant TSS entry structure.
 * @since v0.0.2
 */
void EnableIOPortsOfProcess(Process *p);

/**
 * @brief Disables access to the I/O ports owned by process @p p,
 * usually when that process is blocked and another process is scheduled.
 * @since v0.0.2
 */
void DisableIOPortsOfProcess(Process *p);
