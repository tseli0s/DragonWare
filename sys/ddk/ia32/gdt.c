/**********************************************************************
 * FILE: gdt.c
 * PURPOSE: Global Descriptor Table definition and helpers.
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "gdt.h"

#include <limits.h>
#include <macros.h>
#include <mmutils.h>

#include "assert.h"
#include "ddk/ia32/cpu.h"
#include "ddk/ia32/kcpuid.h"
#include "ktypes.h"
#include "tss.h"
#include "vmm.h"

#define X86_MEMORY_LIMIT   (U32_MAX)
#define DEFAULT_GDT_ALIGN  (16)

/* The values here are documented better in the IDM and
 * https://wiki.osdev.org/Global_Descriptor_Table */

#define GDT_ACCESS_PRESENT (0x80) /* This GDT entry is valid and can be used */
#define GDT_ACCESS_RING3   (0x60) /* This GDT entry is used by programs */
#define GDT_ACCESS_DATA                                                                           \
        (0x10) /* This GDT entry is for program code or data, not for system structures (Like the \
                  TSS) */
#define GDT_ACCESS_EXECUTABLE (0x08) /* This GDT entry is used for executable code */
#define GDT_ACCESS_RW         (0x02) /* This GDT entry can be both read from and written to */
#define GDT_SEGMENT_ACCESSED  (0x01) /* This GDT entry has been accessed */
#define GDT_TYPE_TSS          (0x09)

#define GDT_GRAN_PAGESIZE     (0x80) /* If set, the GDT entry works with page granularity */
#define GDT_ENTRY_32BIT       (0x40) /* If set, this is a 32 bit entry */

/* Defined in gdt_x86.asm */
extern void FlushGDT(u32 ptr);

/* Not necessary, just saw a kernel doing it. */
[[gnu::aligned(DEFAULT_GDT_ALIGN)]]
static GDTEntry gdt[GDT_ENTRIES];

[[gnu::aligned(DEFAULT_GDT_ALIGN)]]
static GDTPointer gdtptr;

[[gnu::aligned(16)]]
static TSSEntry tss0;

[[gnu::aligned(16)]]
static u32 tmp_tss_stack[1024] = {0};

[[gnu::aligned(16)]]
static u32 *tss_stack = tmp_tss_stack;

static void GDTSetGate(int n, u32 base, u32 limit, u8 access, u8 gran) {
        gdt[n].base_low    = (base & U16_MAX);
        gdt[n].base_middle = (base >> BITS_WORD) & 0xFF;
        gdt[n].base_high   = (Byte)((base >> 24) & 0xFF);

        gdt[n].limit_low   = (limit & U16_MAX);
        gdt[n].granularity = (limit >> BITS_WORD) & 0x0F;

        gdt[n].granularity |= gran & 0xF0;
        gdt[n].access = access;
}

static inline void SwitchSysenterStack(u32 esp) {
        __asm__ volatile(
                /* EDX = 0 because we only support 32 bit addresses for now
                (High 32 bits of a 64 bit integer go in edx) */
                "wrmsr"
                :
                : "c"(0x175), /* That's IA32_SYSENTER_ESP. */
                  "d"(0),     /* High bits can be zeroed out in 32 bit mode */
                  "a"(esp)
                : "memory" /* Prevent compiler reordering */
        );
}

void GDTInit(void) {
        ZeroMemory(gdt);
        ZeroMemory(tss_stack);

        gdtptr.limit = (sizeof(GDTEntry) * GDT_ENTRIES) - 1;
        gdtptr.base  = (u32)&gdt;

        GDTSetGate(0, 0, 0, 0, 0);
        GDTSetGate(1, 0, X86_MEMORY_LIMIT,
                   GDT_ACCESS_PRESENT | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_DATA,
                   GDT_ENTRY_32BIT | GDT_GRAN_PAGESIZE); /* Kernel code */
        GDTSetGate(2, 0, X86_MEMORY_LIMIT, GDT_ACCESS_PRESENT | GDT_ACCESS_DATA | GDT_ACCESS_RW,
                   GDT_ENTRY_32BIT | GDT_GRAN_PAGESIZE); /* Kernel data */
        GDTSetGate(3, 0, X86_MEMORY_LIMIT,
                   GDT_ACCESS_PRESENT | GDT_ACCESS_DATA | GDT_ACCESS_EXECUTABLE | GDT_ACCESS_RING3,
                   GDT_ENTRY_32BIT | GDT_GRAN_PAGESIZE); /* User code */
        GDTSetGate(4, 0, X86_MEMORY_LIMIT,
                   GDT_ACCESS_PRESENT | GDT_ACCESS_RW | GDT_ACCESS_DATA | GDT_ACCESS_RING3,
                   GDT_ENTRY_32BIT | GDT_GRAN_PAGESIZE); /* User data */

        /* This part is for the TSS that requires some different code but
         * whatever*/
        kzeromem(&tss0, sizeof(TSSEntry));

        /* Fallback stack, each process will remove this after a while */
        tss0.ss0        = SEL_DATA_KERNEL;
        tss0.esp0       = (u32)(((uintptr_t)tss_stack) + sizeof(tmp_tss_stack));
        tss0.iomap_base = sizeof(tss0);
        GDTSetGate(5, (u32)&tss0, sizeof(tss0) - 1, GDT_ACCESS_PRESENT | GDT_TYPE_TSS, 0x00);

        FlushGDT((u32)&gdtptr);

        __ltr(5 << 3);
        /* Needs to be run once for sysenter support */
        SelectKernelStack(tss0.esp0);
}

void SelectKernelStack(uintptr_t stack_top) {
        kassert(likely(inrange(stack_top, KERNEL_VM_BASE, X86_MEMORY_LIMIT)));
        tss_stack = (u32 *)stack_top;
        tss0.esp0 = stack_top;
#ifndef _LEGACY_SUPPORT
        if (likely(x86FeatureSupported(X86_SYSENTER))) SwitchSysenterStack(stack_top);
#endif /* _LEGACY_SUPPORT */
}
