/**********************************************************************
 * FILE: kmalloc.h
 * PURPOSE: Slab allocator implementation for heap allocation purposes
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Allocate a block of kernel memory.
 * @param size Number of bytes to allocate.
 * @returns Pointer to the allocated memory, or NULL on failure.
 */
void *kmalloc(Size size);

/**
 * @brief Allocate and zero-initialize a block of kernel memory.
 * @param size Number of Bytes to allocate.
 * @returns Pointer to the allocated memory, or NULL on failure.
 */
void *kzalloc(Size size);

/**
 * @brief Free a previously allocated block of kernel memory.
 * @param ptr Pointer to the memory block to free. Must have been returned by kmalloc or kzalloc.
 */
void kfree(void *ptr);
