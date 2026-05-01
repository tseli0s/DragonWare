/**********************************************************************
 * FILE: paging.h
 * PURPOSE: x86 paging constants and flags
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/**
 * @brief Number of entries in a page table.
 * @details Each page table contains 1024 entries when using 4 KiB pages on x86.
 */
#define ENTRIES_PER_TABLE   (1024)

/**
 * @brief Number of entries in a page directory.
 * @details Each page directory contains 1024 entries when using 4 KiB pages on x86.
 */
#define ENTRIES_PER_DRCTR   (1024) /* Directory entry count */

/**
 * @brief Page table entry flag: present.
 * @details Indicates that the page is present in physical memory.
 */
#define PAGE_PRESENT        (0x001)

/**
 * @brief Page table entry flag: read/write.
 * @details If set, the page is writable; otherwise it is read-only.
 */
#define PAGE_RW             (0x002)

/**
 * @brief Page table entry flag: user/supervisor.
 * @details If set, the page is accessible from user mode; otherwise only supervisor mode.
 */
#define PAGE_USER           (0x004)

/**
 * @brief Page table entry flag: write-through caching.
 * @details Enables write-through caching for the page.
 */
#define PAGE_WRITETHROUGH   (0x008)

/**
 * @brief Page table entry flag: cache disabled.
 * @details Disables caching for the page.
 */
#define PAGE_CACHE_DISABLED (0x010)

/**
 * @brief Page table entry flag: accessed.
 * @details Set by the CPU when the page has been read or written.
 */
#define PAGE_ACCESSED       (0x020)

/**
 * @brief Page table entry flag: dirty.
 * @details Set by the CPU when the page has been written to.
 */
#define PAGE_DIRTY          (0x040)

/**
 * @brief Global page (ie. Not flushed from the TLB when switching page directories)
 * @note Only use this for the kernel, which is always at a fixed virtual address.
 */
#define PAGE_GLOBAL         (0x0100)
