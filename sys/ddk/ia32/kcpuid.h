/**********************************************************************
 * FILE: kcpuid.h
 * PURPOSE: CPUID instruction helpers and exports
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define CPUID_MAX_VENDOR    13 /* Twelve characters and a null Byte */
#define CPUID_MAX_SIGNATURE 13 /* >> */

#include <ktypes.h>

#define CPUID_LEAF_GETVNDR 0x0
#define CPUID_LEAF_GETFEAT 0x1
#define CPUID_LEAF_GETTLB  0x2
#define CPUID_LEAF_GETTOPO 0xB

typedef enum _x86Features {
        X86_FPU = 0,
        X86_VME,
        X86_DE,
        X86_PSE,
        X86_TSC,
        X86_MSR,
        X86_PAE,
        X86_MCE,
        X86_CMPXCHG8B, /* What the hell is that I forgot */
        X86_APIC,
        X86_RESERVED0, /* Don't test this bit */
        X86_SYSENTER,
        X86_MTRR,
        X86_PGE,
        X86_MCA,
        X86_CMOV,
        X86_PAT,
        X86_PSE36,
        X86_PSN,
        X86_CLFLUSH,
        X86_RESERVED1, /* Not this one either */
        X86_DBGSTORE,
        X86_ACPI,
        X86_MMX,
        X86_FXSR,
        X86_SSE,
        X86_SSE2,
        X86_SLFSNOOP,
        X86_HTT,
        X86_TM,
        X86_RESERVED2, /* Or this one */
        X86_PBE
} x86Features;

typedef struct _CPUData {
        Bool supported;
        char vendor[CPUID_MAX_VENDOR];
        u32  features;
} CPUData;

/* unused, let it die lmao */
static inline void kcpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx,
                          unsigned int *ecx, unsigned int *edx) {
        __asm__ volatile("cpuid"
                         : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                         : "a"(leaf), "c"(0));
}
