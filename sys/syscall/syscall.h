/**********************************************************************
 * FILE: syscall.h
 * PURPOSE: System call definition numbers
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#define SYSCALL_IDENTIFY        (0)
#define SYSCALL_EXIT            (1)
#define SYSCALL_YIELD           (2)
#define SYSCALL_KLOG            (3)
#define SYSCALL_RAISE_IOPL      (4)
#define SYSCALL_SEND            (5)
#define SYSCALL_RECEIVE         (6)
#define SYSCALL_TICK_SINCE_BOOT (7)
#define SYSCALL_CREATE_OBJECT   (8)
#define SYSCALL_INVOKE_OBJECT   (9)
#define SYSCALL_DELETE_OBJECT   (10)

/* Only define those for the DragonWare kernel */
#if __DRAGONWARE_SYS__
#include <ktypes.h>

#include "ddk/ia32/interrupts.h"

/**
 * @brief Registers passed in every system call to be modified.
 * These registers are used to pass arguments in the kernel and store the
 * return value.
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _SystemCallFrame {
        u32 ebx, esi, edi, ebp; /* Arguments 0-3 of every system call */
        u32 eax;                /* System call number */
        u32 useresp; /* May be used in the future, to copy parameters from the user stack if
                         necessary. */
        u32 eflags;  /* Used for _DWRaiseIOPL only */
} SystemCallFrame;

/**
 * @brief Converts an @ref InterruptStackFrame to a @ref SystemCallFrame that is used
 * whenever system calls are triggered by software interrupts (int 0x60).
 * @since v0.0.2
 */
static inline void SyscallFrameFromInterrupt(InterruptStackFrame *iframe, SystemCallFrame *sframe) {
        sframe->ebx     = iframe->ebx;
        sframe->esi     = iframe->esi;
        sframe->edi     = iframe->edi;
        sframe->ebp     = iframe->ebp;
        sframe->eax     = iframe->eax;
        sframe->useresp = iframe->useresp;
        sframe->eflags  = iframe->eflags;
}

/**
 * @brief Returns a @ref SystemCallFrame from an @ref InterruptStackFrame.
 * @since v0.0.2
 */
SystemCallFrame SystemCallFrameFromInterrupt(InterruptStackFrame *frame);

/* Native DragonWare syscall, we implement our own APIs here */
InterruptStackFrame *DragonWareSyscall(InterruptStackFrame *regs);

/* POSIX syscall. We currently don't implement them, but may do so for compatibility in the future.
 * You know, porting stuff easier. */
InterruptStackFrame *POSIXSyscall(InterruptStackFrame *regs);
#endif /* __DRAGONWARE_SYS__ */
