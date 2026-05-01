/**********************************************************************
 * FILE: panic.h
 * PURPOSE: FatalError() implementation for critical runtime errors
 * PROJECT: DragonWare Kernel
 * DATE: 09-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <macros.h>

/**
 * @brief Halt the system and display a fatal error message.
 * @details This function does not return. It typically clears the framebuffer
 * (or otherwise resets display state), prints the provided message, and then
 * halts the CPU or enters an infinite loop.
 * @param msg Null-terminated format string describing the error condition.
 * @returns Never (May even forcefully reset the machine)
 */
[[noreturn, gnu::nonnull(1)]]
void FatalError(const char *msg, ...);

#ifdef __i386__
#include "ddk/ia32/interrupts.h"
/**
 * @brief Halt the system and display a fatal error message, while also dumping the given @p
 * stack_frame
 * @param stack_frame The stack frame to dump.
 * @param msg The format string to write. Must be null-terminated.
 * @returns Never (May even forcefully reset the machine)
 * @sa FatalError
 */
[[noreturn, gnu::nonnull(1, 2)]]
void FatalErrorWithStackFrame(InterruptStackFrame *stack_frame, const char *msg, ...);
#endif /* __i386__ */
