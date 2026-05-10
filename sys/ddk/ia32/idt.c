/**********************************************************************
 * FILE: idt.c
 * PURPOSE: Interrupt Descriptor Table (IA32) implementation
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "idt.h"

#include <log.h>
#include <macros.h>
#include <panic.h>
#include <power.h>

#include "exception.h"
#include "gdt.h"
#include "interrupts.h"
#include "irq.h"
#include "isr.h"
#include "syscall/syscall.h"

#define RING0_FLAG_EXCEPTION (0x8E)
#define RING0_FLAG_TRAP      (0x8F)
#define RING3_FLAG_TRAP      (0xEE)

extern void SyscallInterruptRoutine(void);

static IDTEntry   idt[IDT_ENTRIES] = {0};
static IDTPointer idtptr           = {0};

static inline void __lidt(u32 idtp) {
        __asm__ volatile(
                "mov %0, %%eax\n"
                "lidt (%%eax)" ::"r"(idtp)
                : "eax");
}

void IDTAddGate(int n, u32 base, u16 selector, u16 flags) {
        idt[n].base_low  = base & 0xFFFF;
        idt[n].base_high = (u16)((base >> 16) & 0xFFFF);
        idt[n].selector  = selector;
        idt[n].zero      = 0;
        idt[n].flags     = (Byte)flags;
}

static void InterruptServiceFallbackHandler(void) {
        LogMessage(LOG_WARNING,
                   "InterruptServiceFallbackHandler invoked; We are not handling an interrupt we "
                   "should.");
}

static void IRQInstall(void) {
        IDTAddGate(IRQ_BASE + 0, (u32)InterruptServiceRoutine32, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 1, (u32)InterruptServiceRoutine33, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 2, (u32)InterruptServiceRoutine34, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 3, (u32)InterruptServiceRoutine35, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 4, (u32)InterruptServiceRoutine36, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 5, (u32)InterruptServiceRoutine37, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 6, (u32)InterruptServiceRoutine38, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 7, (u32)InterruptServiceRoutine39, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 8, (u32)InterruptServiceRoutine40, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 9, (u32)InterruptServiceRoutine41, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 10, (u32)InterruptServiceRoutine42, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 11, (u32)InterruptServiceRoutine43, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 12, (u32)InterruptServiceRoutine44, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 13, (u32)InterruptServiceRoutine45, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 14, (u32)InterruptServiceRoutine46, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
        IDTAddGate(IRQ_BASE + 15, (u32)InterruptServiceRoutine47, SEL_CODE_KERNEL,
                   RING0_FLAG_EXCEPTION);
}

static void InstallInterruptServiceRoutines(void) {
        IDTAddGate(0, (u32)InterruptServiceRoutine0, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(1, (u32)InterruptServiceRoutine1, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(2, (u32)InterruptServiceRoutine2, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(3, (u32)InterruptServiceRoutine3, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(4, (u32)InterruptServiceRoutine4, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(5, (u32)InterruptServiceRoutine5, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(6, (u32)InterruptServiceRoutine6, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(7, (u32)InterruptServiceRoutine7, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(8, (u32)InterruptServiceRoutine8, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(9, (u32)InterruptServiceRoutine9, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(10, (u32)InterruptServiceRoutine10, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(11, (u32)InterruptServiceRoutine11, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(12, (u32)InterruptServiceRoutine12, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(13, (u32)InterruptServiceRoutine13, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(14, (u32)InterruptServiceRoutine14, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(15, (u32)InterruptServiceRoutine15, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(16, (u32)InterruptServiceRoutine16, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(17, (u32)InterruptServiceRoutine17, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(18, (u32)InterruptServiceRoutine18, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(19, (u32)InterruptServiceRoutine19, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(20, (u32)InterruptServiceRoutine20, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(21, (u32)InterruptServiceRoutine21, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(22, (u32)InterruptServiceRoutine22, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(23, (u32)InterruptServiceRoutine23, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(24, (u32)InterruptServiceRoutine24, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(25, (u32)InterruptServiceRoutine25, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(26, (u32)InterruptServiceRoutine26, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(27, (u32)InterruptServiceRoutine27, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(28, (u32)InterruptServiceRoutine28, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(29, (u32)InterruptServiceRoutine29, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(30, (u32)InterruptServiceRoutine30, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);
        IDTAddGate(31, (u32)InterruptServiceRoutine31, SEL_CODE_KERNEL, RING0_FLAG_EXCEPTION);

        for (int i = IRQ_BASE; i < IDT_ENTRIES; i++)
                IDTAddGate(i, (u32)InterruptServiceFallbackHandler, SEL_CODE_KERNEL,
                           RING0_FLAG_EXCEPTION);
        IDTAddGate(SYSCALL_NO, (u32)InterruptServiceRoutine0x60, SEL_CODE_KERNEL, RING3_FLAG_TRAP);
        IDTAddGate(SYSCALL_NO + 0x20, (u32)InterruptServiceRoutine0x80, SEL_CODE_KERNEL,
                   RING3_FLAG_TRAP);
        IRQInstall();
}

/* I should probably find a better name for this function */
[[gnu::hot]]
static inline void CopySyscallState(SystemCallFrame *state, InterruptStackFrame *frame) {
        frame->eax    = state->eax;    /* Return value */
        frame->ebx    = state->ebx;    /* Argument 0 that (may) be returned from the kernel */
        frame->esi    = state->esi;    /* Argument 1 that (may) be returned from the kernel */
        frame->edi    = state->edi;    /* Argument 2 that (may) be returned from the kernel */
        frame->ebp    = state->ebp;    /* Argument 3 that (may) be returned from the kernel */
        frame->eflags = state->eflags; /* eflags that may have been modified by the kernel. For now,
                                          this is only relevant for _DWRaiseIOPL()  */
}

[[gnu::hot]]
void InterruptServiceHandler(InterruptStackFrame *stack_frame) {
        if (stack_frame->int_no == SYSCALL_NO) {
                SystemCallFrame f;
                SyscallFrameFromInterrupt(stack_frame, &f);
                DragonWareSyscall(&f);
                CopySyscallState(&f, stack_frame);
                return;
        }
        if (stack_frame->int_no == SYSCALL_NO + 0x20) {
                SystemCallFrame f;
                SyscallFrameFromInterrupt(stack_frame, &f);
                POSIXSyscall(&f);
                CopySyscallState(&f, stack_frame);
                return;
        }

        if (inrange(stack_frame->int_no, EXCEPTION_BASE, MAX_EXCEPTIONS - 1))
                _DWExceptionHandler(stack_frame);
        else if (inrange(stack_frame->int_no, IRQ_BASE, (IRQ_BASE + MAX_IRQ) - 1))
                IRQHandlerCallback(stack_frame);
        else
                InterruptServiceFallbackHandler();
}

void IDTInit(void) {
        InstallInterruptServiceRoutines();

        idtptr.limit = (sizeof(IDTEntry) * IDT_ENTRIES) - 1;
        idtptr.base  = (u32)&idt;

        __lidt((u32)&idtptr);
        IRQInit();
}

static int  critical_counter        = 0;
static Bool interrupts_were_enabled = false;

void PushCritical(void) {
        if (critical_counter == 0) interrupts_were_enabled = InterruptsEnabled();
        DisableInterrupts();

        critical_counter++;
}

void PopCritical(void) {
        if (critical_counter <= 0)
                FatalError("Attempted to leave critical section without being in one!");

        critical_counter--;
        if (critical_counter == 0 && interrupts_were_enabled) EnableInterrupts();
}
