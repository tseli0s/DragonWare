/**********************************************************************
 * FILE: mmutils.h
 * PURPOSE: Memory manipulation/reading functions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* The extern functions here are written in assembly to squeeze every single bit
 * of performance. */

/**
 * @brief Copy a block of memory.
 * @param dest Destination buffer.
 * @param src Source buffer.
 * @param n Number of Bytes to copy.
 * @returns Pointer to the destination buffer.
 * @note Behavior is undefined if the source and destination overlap (Which is the reason behind the
 * restrict keyword being used). This isn't the default in most implementations, but it is here
 * because I wrote a very nice @ref memmove to use
 */
extern void *memcpy(void *restrict dest, const void *restrict src, Size n);

/**
 * @brief Set a block of memory to zero.
 * @param dest Destination buffer.
 * @param size Number of Bytes to clear.
 */
extern void kzeromem(void *dest, Size size);

/**
 * @brief Fill a block of memory with a Byte value.
 * @param obj Destination buffer.
 * @param value Byte value to set.
 * @param size Number of Bytes to set.
 * @returns Pointer to the destination buffer.
 */
extern void *memset(void *obj, int value, Size size);

/**
 * @brief Copy a block of memory with overlap support.
 * @param dest Destination buffer.
 * @param src Source buffer.
 * @param n Number of Bytes to copy.
 * @returns Pointer to the destination buffer.
 * @note This function correctly handles overlapping source and destination regions.
 */
void *memmove(void *dest, const void *src, Size n);

/**
 * @brief Compare two blocks of memory.
 * @param buf First buffer.
 * @param cmp Second buffer.
 * @param n Number of Bytes to compare.
 * @returns 0 if equal, negative if buf < cmp, positive if buf > cmp.
 */
int memcmp(const void *buf, const void *cmp, Size n);
