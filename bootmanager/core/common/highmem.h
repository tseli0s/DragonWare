/**********************************************************************
 * FILE: highmem.c
 * PURPOSE: High memory allocation code used for modules and large structures
 * PROJECT: DragonWare Boot Manager
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#define HIGH_MEMORY_START (0x100000)

/**********************************************************************************************
 * Small notice: The bootloader doesn't use virtual memory in any way (not even identity mapped).
 * This means there's no need for mapping/unmapping any allocations done here (or anywhere else,
 * really, but this is probably the place that you wouldn't expect to be mapped).
 **************************************************************************************************/

/**
 * @brief Initialize the high memory allocator.
 * @note Call this only after probing all the memory regions in the machine, this function needs to
 * know about all memory regions to select the best region to allocate from.
 */
void AllocHighInit(void);

/**
 * @brief Set the pool size of the high memory allocator from @p start to @p end.
 * @param[in] start New start @b physical address of the pool. Only used if the new address is
 * higher than the old one. @b MUST be page-aligned.
 * @param[in] end New end @b physical address of the pool. If zero, this argument is unused. @b MUST
 * be page-aligned.
 */
void AllocHighBreakAt(uintptr_t start, uintptr_t end);

/**
 * @brief Allocates a contiguous memory block of exactly @p n_pages amount of pages wide and returns
 * a pointer to the start of the allocated block.
 * @param[in] n_pages Amount of pages to allocate. Must be >0.
 * @return A pointer to the start of the memory allocated, or NullPointer if there's not enough
 * memory or @p n_pages is 0.
 */
void *AllocateHighMemory(Size n_pages);

/**
 * @brief Frees up a memory block allocated by @ref AllocateHighMemory
 * @warning Currently no-op. This is just a placeholder for the future.
 * @param[in] addr Starting address of the allocation returned by @ref AllocateHighMemory
 * @note If addr is NullPointer, this function does not do anything.
 */
void FreeHighMemory(void *addr);
