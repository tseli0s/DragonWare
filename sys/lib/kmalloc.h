/**********************************************************************
 * FILE: kmalloc.h
 * PURPOSE: Slab allocator for DragonWare's kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Allocate a block of kernel memory.
 * @param size Number of Bytes to allocate.
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

/**
 * @brief Allocate a single physical page and map it into the higher-half virtual address space.
 * @details This function returns a virtual address to the mapped page. The page is allocated
 * from the kernel's physical memory allocator and mapped using the kernel's page tables.
 * @returns Virtual address of the allocated page, or NULL on failure.
 */
void *AllocateVirtualPage(void);

/**
 * @brief Free a page previously allocated with @ref AllocateVirtualPage.
 * @param physaddr Virtual address returned by @ref AllocateVirtualPage.
 */
void FreeVirtualPage(void *physaddr);
