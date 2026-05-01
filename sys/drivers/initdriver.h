/**********************************************************************
 * FILE: initdriver.c
 * PURPOSE: Driver loading/initialization routines
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Initialize and enable built-in kernel drivers.
 * @details This function brings the kernel's built-in device drivers online, performing
 * any required initialization and registration steps.
 * @returns STATUS_OK on success, negative value on failure.
 */
Status BringBuiltinDriversOnline(void);
