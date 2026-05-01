/**********************************************************************
 * FILE: ctxswitch.h
 * PURPOSE: Context switching exports for the kernel
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "task/process.h"

/**
 * @brief Performs a low-level context switch between two execution thread stacks.
 * @details This function saves the current CPU state (callee-saved registers) onto the
 * current stack, switches the stack pointer (ESP) to the new thread's stack,
 * and restores the saved state of the new thread. Scratch registers are automatically saved by the
 * compiler, so they aren't taken care of here.
 * @note The memory layout of the stack must match the order of pushes/pops
 * expected by this function (EBP, EBX, ESI, EDI). This is usually handled by @ref AllocateThread
 * @param[in,out] old A @b pointer to the storage location for the current thread's ESP.
 * @param[in] new The @b value of the stack pointer for the thread being switched to.
 * @returns This function returns to the caller of the @b new thread.
 */
[[gnu::nonnull, gnu::hot, gnu::cdecl]]
extern void SwapThreadStack(u32 *old, u32 new);

/**
 * @brief Switches the process-specific structures inside the processor to the ones needed by @p
 * nmew
 * @param[in] new The new process to be switched to, must not be NullPointer.
 * @note This doesn't perform a total context switch. It replaces the kernel stack in the TSS and
 * switches address spaces only.
 */
[[gnu::nonnull, gnu::hot]]
void SwapProcess(Process *new);
