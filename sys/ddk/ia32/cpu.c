/**********************************************************************
 * FILE: cpu.c
 * PURPOSE: CPU-specific utilities and helpers
 * PROJECT: DragonWare Kernel
 * DATE: 09-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "cpu.h"

#include <ktypes.h>
#include <macros.h>
#include <panic.h>

#define MAX_CPU_FEATURES (32)

#include <cpuid.h>
#include <mmutils.h>

#include "gdt.h"
#include "idt.h"
#include "kcpuid.h"
#include "log.h"

#define MAX_FEATURE_STRING_SIZE 512
static char feature_str[MAX_FEATURE_STRING_SIZE];

extern u32  __check_cpuid_exists(void);
extern void EnableSysenter(void);

static Status GetCPUIDx86(CPUData *data) {
        if (!data) return STATUS_BAD;
        kzeromem(data, sizeof(*data));

        if (!__check_cpuid_exists()) {
                LogMessage(LOG_ERROR, "cpuid instruction not supported on this host");
                return STATUS_BAD;
        }
        LogMessage(LOG_DEBUG, "cpuid instruction supported on this host");
        data->supported = true;

        unsigned int eax, ebx, ecx, edx;
        __cpuid(CPUID_LEAF_GETVNDR, eax, ebx, ecx, edx);
        /* TODO: is there a faster way to write this WITHOUT crashing the kernel?? (Yes assignment
         * directly from the register values "works" but used to break the kernel in release builds
         * and now I'm scared to go back) */
        data->vendor[0]  = (char)((ebx >> 0) & 0xFF);
        data->vendor[1]  = (char)((ebx >> 8) & 0xFF);
        data->vendor[2]  = (char)((ebx >> 16) & 0xFF);
        data->vendor[3]  = (char)((ebx >> 24) & 0xFF);
        data->vendor[4]  = (char)((edx >> 0) & 0xFF);
        data->vendor[5]  = (char)((edx >> 8) & 0xFF);
        data->vendor[6]  = (char)((edx >> 16) & 0xFF);
        data->vendor[7]  = (char)((edx >> 24) & 0xFF);
        data->vendor[8]  = (char)((ecx >> 0) & 0xFF);
        data->vendor[9]  = (char)((ecx >> 8) & 0xFF);
        data->vendor[10] = (char)((ecx >> 16) & 0xFF);
        data->vendor[11] = (char)((ecx >> 24) & 0xFF);
        data->vendor[12] = '\0';

        __cpuid(CPUID_LEAF_GETFEAT, eax, ebx, ecx, edx);
        data->features = edx;

        return STATUS_OK;
}

static Size append_feature(char *dst, Size remaining, const char *name) {
        Size written = 0;
        while (*name && remaining > 1) {
                *dst++ = *name++;
                remaining--;
                written++;
        }
        if (remaining > 0) {
                *dst = '\0';
        }
        return written;
}

static void LogCPUInfo(void) {
        ZeroMemory(feature_str);
        feature_str[0]  = '\0';
        char *p         = feature_str;
        Size  remaining = MAX_FEATURE_STRING_SIZE;

        CPUData data = {0};
        if (GetCPUIDx86(&data) != 0) FatalError("unable to fetch processor information!");

        if (!data.supported || data.features == 0x0) {
                LogMessage(LOG_ERROR, "Bad processor information, continuing anyways.");
                return;
        }

        u32 featbit = data.features;
        LogMessage(LOG_INFO, "Processor vendor: %s", data.vendor);

        struct {
                u32         mask;
                const char *name;
        } feature_table[] = {
                /* the 1s must be marked with U otherwise the compiler may think we're using bytes here */
                {1U << X86_FPU, "fpu "},
                {1U << X86_VME, "vme "},
                {1U << X86_DE, "de "},
                {1U << X86_PSE, "pse "},
                {1U << X86_TSC, "tsc "},
                {1U << X86_MSR, "msr "},
                {1U << X86_PAE, "pae "},
                {1U << X86_MCE, "mce "},
                {1U << X86_CMPXCHG8B, "cmpxchg8b "},
                {1U << X86_APIC, "apic "},
                {1U << X86_SYSENTER, "sysenter "},
                {1U << X86_MTRR, "mtrr "},
                {1U << X86_PGE, "pge "},
                {1U << X86_MCA, "mca "},
                {1U << X86_CMOV, "cmov "},
                {1U << X86_PAT, "pat "},
                {1U << X86_PSE36, "pse36 "},
                {1U << X86_PSN, "psn "},
                {1U << X86_CLFLUSH, "clflush "},
                {1U << X86_DBGSTORE, "dbgstore "},
                {1U << X86_ACPI, "acpi "},
                {1U << X86_MMX, "mmx "},
                {1U << X86_FXSR, "fxsr "},
                {1U << X86_SSE, "sse "},
                {1U << X86_SSE2, "sse2 "},
                {1U << X86_SLFSNOOP, "slfsnoop "},
                {1U << X86_HTT, "htt "},
                {1U << X86_TM, "tm "},
                {1U << X86_PBE, "pbe "},
        };

        /* This code below doesn't work at all, I have no idea why, let it die for now and we'll fix
         * it one day */
        for (Size i = 0; i < sizeof(feature_table) / sizeof(feature_table[0]); i++) {
                if (featbit & feature_table[i].mask) {
                        Size written = append_feature(p, remaining, feature_table[i].name);
                        p += written;
                        remaining -= written;
                }
        }
        LogMessage(LOG_INFO, "Processor detected features: %s", feature_str);
}

Bool x86FeatureSupported(x86Features feat) {
        u32 eax, ebx, ecx, edx;
        __cpuid(CPUID_LEAF_GETFEAT, eax, ebx, ecx, edx);
        u32 features = edx;
        return (features & (1 << feat));
}

Status ArchInit(void) {
        LogCPUInfo();
        GDTInit();
        IDTInit();

#ifndef _LEGACY_SUPPORT
        if (x86FeatureSupported(X86_SYSENTER)) {
                LogMessage(LOG_INFO,
                           "Fast system call (sysenter/sysexit) supported in this machine, "
                           "enabling support code for it.");
                EnableSysenter();
        } else
#endif /* _LEGACY_SUPPORT */
                LogMessage(LOG_WARNING,
                           "This machine doesn't support modern sysenter/sysexit instructions for "
                           "system calls. The operating system is going to be slower.");
        return STATUS_OK;
}
