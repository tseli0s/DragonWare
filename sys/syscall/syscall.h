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
#include "ddk/ia32/interrupts.h"
/* Native DragonWare syscall, we implement our own APIs here */
InterruptStackFrame *DragonWareSyscall(InterruptStackFrame *regs);

/* POSIX syscall. We currently don't implement them, but may do so for compatibility in the future.
 * You know, porting stuff easier. */
InterruptStackFrame *POSIXSyscall(InterruptStackFrame *regs);
#endif /* __DRAGONWARE_SYS__ */
