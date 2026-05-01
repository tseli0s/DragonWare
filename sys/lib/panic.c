/**********************************************************************
 * FILE: panic.c
 * PURPOSE: FatalError() implementation for critical runtime errors
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "panic.h"

#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <power.h>

#include "ddk/ia32/exception.h"
#include "ddk/ia32/interrupts.h"
#include "iomgr/node.h"
#include "video/output.h"
#include "video/pixels.h"

#define NUMBUF_REG (32)

static Bool panicking = 0;

enum Base { BASE8 = 8, BASE10 = 10, BASE16 = 16 };

typedef struct _BacktraceStackFrame {
        struct _BacktraceStackFrame *ebp;
        u32                          eip;
} BacktraceStackFrame;

/* Utility function to print a string wherever the kernel can print a string */
static void PrintStringToAllOutputs(const char *str) {
        ForEachConsoleDevice({
                DeviceManagerNode *out = curr->node;
                for (unsigned int i = 0; i < strlen(str); i++)
                        out->devtable.ddo->console.WriteSingleChar(out->private_state, str[i]);
        })
}

/* Helper to dump a full backtrace */
static void DumpBacktraceFromFramePointer(void) {
#ifdef __i386__

        BacktraceStackFrame *frame;
        __asm__ volatile("mov %%ebp, %0" : "=r"(frame));
        const unsigned int max_frames_bt = 32;
        for (unsigned int i = 0; i < max_frames_bt && frame != NullPointer; i++) {
                char buf[128];
                snprintf(buf, sizeof(buf), "\t%d: 0x%x\n", i, frame->eip);
                PrintStringToAllOutputs(buf);
                frame = frame->ebp;
        }

#endif /* __i386__ */
}

#ifdef __i386__
static void DumpRegisterValues(void) {
        u32  eax = 0, ebx = 0, ecx = 0, edx = 0, esi = 0, edi = 0, ebp = 0;
        u32  esp_val = 0, eflags_val = 0;
        u32  cr3 = 0, cr4 = 0;
        char buf[NUMBUF_REG];

        __asm__ volatile(
                "mov %%eax, %0\n"
                "mov %%ebx, %1\n"
                "mov %%ecx, %2\n"
                "mov %%edx, %3\n"
                "mov %%esi, %4\n"
                "mov %%edi, %5\n"
                "mov %%ebp, %6\n"
                : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx), "=m"(esi), "=m"(edi), "=m"(ebp)
                :
                : "memory");

        __asm__ volatile("mov %%esp, %0" : "=m"(esp_val) : : "memory");

        __asm__ volatile(
                "pushf\n"
                "pop %0\n"
                : "=m"(eflags_val)
                :
                : "memory");

        __asm__ volatile(
                "mov %%cr3, %%ebx\n"
                "mov %%cr4, %%ecx\n"
                "mov %%eax, %0\n"
                "mov %%ebx, %1\n"
                "mov %%ecx, %1\n"
                : "=m"(cr3), "=m"(cr4)
                :
                : "memory");
        PrintStringToAllOutputs("EAX=0x");
        itoa(eax, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EBX=0x");
        itoa(ebx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("ECX=0x");
        itoa(ecx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EDX=0x");
        itoa(edx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");

        PrintStringToAllOutputs("ESI=0x");
        itoa(esi, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EDI=0x");
        itoa(edi, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EBP=0x");
        itoa(ebp, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("ESP=0x");
        itoa(esp_val, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");

        PrintStringToAllOutputs("EFLAGS=0x");
        itoa(eflags_val, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR2=0x");
        itoa(GetFaultingAddress(), buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR3=0x");
        itoa(cr3, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR4=0x");
        itoa(cr4, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");
}
#endif /* __i386__ */

[[noreturn]]
void FatalError(const char *fmt, ...) {
        DisableInterrupts();
        char    buffer[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);

        if (!panicking) {
                panicking = true;
                ForEachConsoleDevice({
                        DeviceManagerNode *out = curr->node;
                        void               (*SetTextAttributes)(void *, PixelColor, PixelColor) =
                                out->devtable.ddo->console.SetTextAttributes;
                        void (*ResetConsole)(void *) = out->devtable.ddo->console.ResetConsole;
                        if (SetTextAttributes)
                                SetTextAttributes(out->private_state, RedPixel, WhitePixel);
                        if (ResetConsole) ResetConsole(out->private_state);
                });
                PrintStringToAllOutputs("\n");
                PrintStringToAllOutputs("*** FATAL KERNEL ERROR ***\n");
                PrintStringToAllOutputs(buffer);

                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs("Register Dump: \n");
                DumpRegisterValues();
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs(
                        "Backtrace Dump (May be inaccurate depending on compile options):\n");
                DumpBacktraceFromFramePointer();
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs(
                        "Cannot continue; Condition is unrecoverable. Please try the following:\n");
                PrintStringToAllOutputs("* Unplug any new hardware you might've connected\n");
                PrintStringToAllOutputs(
                        "* Use a memory/hardware testing tool to verify your hardware is in good "
                        "health\n");
                PrintStringToAllOutputs(
                        "* Check https://github.com/tseli0s/DragonWare/issues and see if your "
                        "problem appears (If it doesn't, file a bug report)\n");
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs("TOTAL SYSTEM STALL :: YOU CAN REBOOT YOUR MACHINE NOW\n");
        } else
                ForceReboot(); /* TODO: More outputs here */

        StallMachine();
}

#ifdef __i386__

static void DumpRegisterValuesFromStackFrame(InterruptStackFrame *stack_frame) {
        u32 cr2 = 0;
        u32 cr3 = 0;
        u32 cr4 = 0;
        __asm__ volatile(
                "mov %%cr2, %%eax\n"
                "mov %%cr3, %%ebx\n"
                "mov %%cr4, %%ecx\n"
                "mov %%eax, %0\n"
                "mov %%ebx, %1\n"
                "mov %%ecx, %2\n"
                : "=m"(cr2), "=m"(cr3), "=m"(cr4)
                :
                : "memory");

        char                 buf[NUMBUF_REG];
        InterruptStackFrame *f = stack_frame; /* Faster typing, nothing more */

        PrintStringToAllOutputs("EAX=0x");
        itoa(f->eax, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EBX=0x");
        itoa(f->ebx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("ECX=0x");
        itoa(f->ecx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EDX=0x");
        itoa(f->edx, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");

        PrintStringToAllOutputs("ESI=0x");
        itoa(f->esi, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EDI=0x");
        itoa(f->edi, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EBP=0x");
        itoa(f->ebp, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("ESP=0x");
        itoa(f->esp, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");

        PrintStringToAllOutputs("EFLAGS=0x");
        itoa(f->eflags, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");
        PrintStringToAllOutputs("EIP=0x");
        itoa(f->eip, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR2=0x");
        itoa(cr2, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR3=0x");
        itoa(cr3, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CR4=0x");
        itoa(cr4, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("CS=0x");
        itoa(stack_frame->cs, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs(" ");

        PrintStringToAllOutputs("DS=0x");
        itoa(stack_frame->ds, buf, BASE16);
        PrintStringToAllOutputs(buf);
        PrintStringToAllOutputs("\n");
}

[[noreturn]]
void FatalErrorWithStackFrame(InterruptStackFrame *stack_frame, const char *msg, ...) {
        DisableInterrupts();
        char    buffer[512];
        va_list args;
        va_start(args, msg);
        vsnprintf(buffer, sizeof(buffer), msg, args);
        va_end(args);
        if (!panicking) {
                panicking = true;
                ForEachConsoleDevice({
                        DeviceManagerNode *out = curr->node;
                        void               (*SetTextAttributes)(void *, PixelColor, PixelColor) =
                                out->devtable.ddo->console.SetTextAttributes;
                        void (*ResetConsole)(void *) = out->devtable.ddo->console.ResetConsole;
                        if (SetTextAttributes)
                                SetTextAttributes(out->private_state, RedPixel, WhitePixel);
                        if (ResetConsole) ResetConsole(out->private_state);
                });
                PrintStringToAllOutputs("\n");
                PrintStringToAllOutputs("*** FATAL KERNEL ERROR ***\n");
                PrintStringToAllOutputs(buffer);

                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs("Register Dump: \n");
                DumpRegisterValuesFromStackFrame(stack_frame);
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs(
                        "Backtrace Dump (May be inaccurate depending on compile options):\n");
                DumpBacktraceFromFramePointer();
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs(
                        "Cannot continue; Condition is unrecoverable. Please try the following:\n");
                PrintStringToAllOutputs("* Unplug any new hardware you might've connected\n");
                PrintStringToAllOutputs(
                        "* Use a memory/hardware testing tool to verify your hardware is in good "
                        "health\n");
                PrintStringToAllOutputs(
                        "* Check https://github.com/tseli0s/DragonWare/issues and see if your "
                        "problem appears (If it doesn't, file a bug report)\n");
                PrintStringToAllOutputs("\n\n");
                PrintStringToAllOutputs("TOTAL SYSTEM STALL :: YOU CAN REBOOT YOUR MACHINE NOW\n");
        } else
                ForceReboot();

        StallMachine();
        __builtin_unreachable();
}
#endif /* __i386__ */
