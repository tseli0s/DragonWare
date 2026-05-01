/**********************************************************************
 * FILE: wfi.h
 * PURPOSE: Main kernel loop (Waiting for interrupts and putting the machine in a low power state
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <macros.h>

/**
 * @brief Begin the kernel's infinite main loop.
 * @details This function does three major things, in an infinite loop:
 * 1. Enables the interrupts for the processor, allowing things like the timer to start
 * firing. 2. Instructs the processor to enter a low power mode and not try to execute further
 * instructions. 3. Waits for interrupts to come from hardware and handle them.
 * Without this function, the kernel would return, which should never happen (There are also guards
 * for that in the bootstrap code).
 * Interrupts are handled elsewhere; This function simply enables them and waits for them to come.
 * @returns Never. If it returns, two things may happen: Either the kernel will panic, or the entire
 * system will freeze. The only way it can return is some sort of stack corruption.
 */
[[noreturn]]
void WaitForInterrupts(void);
