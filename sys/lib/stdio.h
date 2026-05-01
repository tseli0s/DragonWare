/**********************************************************************
 * FILE: stdio.h
 * PURPOSE: Standard I/O libc functions for the kernel
 * PROJECT: DragonWare Freestanding Library
 * DATE: 23-02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/**
 * @brief Formats and prints a string to the default kernel output (Or, depending on the
 * configuration, all console output devices)
 * @param fmt The formatting string to use
 * @return Amount of bytes written to the standard output
 */
int printf(const char *fmt, ...);

/**
 * @brief Allocates a new buffer on the heap and copies @p src to it.
 * @param src The string to duplicate
 * @return The heap allocated duplicated string.
 */
char *kstrdup(const char *src);
