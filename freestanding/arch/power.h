/**********************************************************************
 * FILE: power.h
 * PURPOSE: Generic x86 power management functions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <macros.h>

/**
 * @brief Forcefully resets the machine using a triple-fault (Intentional crash)
 * @returns Never
 */
[[noreturn]]
extern void ForceReboot(void);

/**
 * @brief Stalls the machine indefinitely by disabling all interrupts and entering an infinite loop.
 * @returns Never
 */
[[noreturn]]
extern void StallMachine(void);
