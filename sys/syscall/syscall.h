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

#define SYSCALL_IDENTIFY         (0)
#define SYSCALL_EXIT             (1)
#define SYSCALL_YIELD            (2)
#define SYSCALL_KLOG             (3)
#define SYSCALL_REQUEST_PORTS    (4) /* Replaced the old _DWRaiseIOPL syscall */
#define SYSCALL_SEND             (5)
#define SYSCALL_RECEIVE          (6)
#define SYSCALL_TICK_SINCE_BOOT  (7)
#define SYSCALL_CREATE_OBJECT    (8)
#define SYSCALL_INVOKE_OBJECT    (9)
#define SYSCALL_DELETE_OBJECT    (10)
#define SYSCALL_TRANSLATE_HANDLE (11)
#define SYSCALL_SYSTEM_QUERY     (12)

/* Only define those for the DragonWare kernel */
#if __DRAGONWARE_SYS__
#include <ktypes.h>

#include "ddk/ia32/interrupts.h"

/**
 * @brief Call into @p __syscall and store the return value of this call into @p __frame which will
 * then be received back from the userland
 * @param[in] __frame The @ref SystemCallFrame to return the value to. The return value of @p
 * __syscall will be returned there and read from userland.
 * @param[in] __syscall Function pointer to the system call to perform and return the value of.
 * @note Arguments to @p __syscall are passed through macro variadic arguments (__VA_ARGS__). System
 * calls that do not return any value should not use this macro.
 * @since v0.0.2
 * @sa SystemCallFrame
 */
#define ReturnFromSystemCall(__frame, __syscall, ...)           \
        do {                                                    \
                (__frame)->eax = (u32)(__syscall)(__VA_ARGS__); \
        } while (0)

/**
 * @brief Registers passed in every system call to be modified.
 * These registers are used to pass arguments in the kernel and store the
 * return value.
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _SystemCallFrame {
        u32 ebx, esi, edi, ebp; /* Arguments 0-3 of every system call */
        u32 eax;                /* System call number */
} SystemCallFrame;

/**
 * @brief Converts an @ref InterruptStackFrame to a @ref SystemCallFrame that is used
 * whenever system calls are triggered by software interrupts (int 0x60).
 * @since v0.0.2
 */
[[gnu::hot]]
static inline void SyscallFrameFromInterrupt(InterruptStackFrame *iframe, SystemCallFrame *sframe) {
        sframe->ebx = iframe->ebx;
        sframe->esi = iframe->esi;
        sframe->edi = iframe->edi;
        sframe->ebp = iframe->ebp;
        sframe->eax = iframe->eax;
}

/* Native DragonWare syscall, we implement our own APIs here */
void DragonWareSyscall(SystemCallFrame *frame);

#endif /* __DRAGONWARE_SYS__ */
