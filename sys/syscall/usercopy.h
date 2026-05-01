/**********************************************************************
 * FILE: usercopy.h
 * PURPOSE: Userland to kernel buffer copying safety guards
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/**
 * @brief Copies @p n_bytes of data from userspace address @p src data into @p dest address,
 * ensuring that the userspace process doesn't try to copy data it shouldn't into kernel space.
 * @param[in] dest The destination buffer. Must be in kernel space.
 * @param[in] src The source to copy from. Must be in user space.
 * @param[in] n_bytes Amount of bytes to copy.
 * @return @ref STATUS_OK if the copy succeeded, @ref STATUS_BAD if the pointers cannot be trusted.
 */
[[gnu::nonnull(1, 2)]]
Status CopyFromUser(void *restrict dest, const void *restrict src, Size n_bytes);

/**
 * @brief Copies @p n_bytes of data from kernel space address @p src data into @p dest user address,
 * ensuring that the userspace process doesn't try to copy data it shouldn't from kernel space.
 * @param[in] dest The destination buffer. Must be in user space.
 * @param[in] src The source to copy from. Must be in kernel space.
 * @param[in] n_bytes Amount of bytes to copy.
 * @returns @ref STATUS_OK if the copy succeeded, @ref STATUS_BAD if the pointers cannot be trusted.
 */
[[gnu::nonnull(1, 2)]]
Status CopyToUser(void *restrict dest, const void *restrict src, Size n_bytes);
