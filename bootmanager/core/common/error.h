/**********************************************************************
 * FILE: error.h
 * PURPOSE: FatalError() implementation for bootloader purposes, ported from the kernel.
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <macros.h>

/**
 * @brief Raise a fatal error to the user, indicating that the error that occurred cannot be
 * recovered from.
 * @param[in] msg The error message to show. Must not be @ref NullPointer
 * @returns Never.
 */
[[noreturn, gnu::nonnull(1)]]
void FatalError(const char *msg, ...);

/**
 * @brief Display an error message to the user that can be recovered from.
 * @param[in] msg The message to display, must be nonnull.
 */
[[gnu::nonnull(1)]]
void RecoverableError(const char *msg, ...);
