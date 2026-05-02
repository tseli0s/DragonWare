/**********************************************************************
 * FILE: vmm.h
 * PURPOSE: Virtual memory manager and implementation for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/**
 * @brief Base address of the kernel in virtual memory.
 * @details The kernel is mapped at a fixed virtual address to provide a consistent
 * view of kernel memory across different processes.
 */
#define KERNEL_VM_BASE          (0xC0000000UL)

/**
 * @brief Base address of the kernel in physical memory (linker-mapped address).
 * @details This is the physical load memory address (LMA) where the kernel is placed.
 */
#define KERNEL_LMA_BASE         (0x00100000UL)

/**
 * @brief Index of the first kernel page table within the page directory
 * @note To be used for shallow copies of the kernel virtual address space.
 */
#define KERNEL_PD_INDEX         (PD_INDEX(KERNEL_VM_BASE))

/** @brief Page directory index (The location of a page table within a page directory) */
#define PD_INDEX(x)             ((((uintptr_t)(x) >> 22) & 0x3FF))
/** @brief Location of a page table entry within a page table (Location of a 4KB-mapped page) */
#define PT_INDEX(x)             ((((uintptr_t)(x) >> 12) & 0x3FF))
/** @brief Bit masking of the page frame without including other bits */
#define PAGE_FRAME_MASK         (u32)(~(PAGE_SIZE - 1))

/**
 * @brief Maximum amount of @ref PageTableEntry a page directory can have
 * @note This assumes no PAE is used.
 */
#define MAX_PD_ENTRIES          (1024)

/** @brief Maximum amount of entries a @ref PageTableEntry can have */
#define MAX_PT_ENTRIES          (1024)

/**
 * @brief Offset between kernel virtual and physical base addresses.
 * @details Used to translate between physical and virtual addresses within the kernel
 * mapping region.
 */
#define VIRT_OFFSET             (KERNEL_VM_BASE - KERNEL_LMA_BASE)

/**
 * @brief A pointer to the current page directory used by the CPU.
 * The trick here is called recursive paging, see @file vmm.c to understand.
 */
#define CURRENT_PAGE_DIRECTORY  ((PageDirectory *)0xFFFFF000)

/**
 * @brief A pointer to the first page table within the virtual address space.
 * @sa CURRENT_PAGE_DIRECTORY
 */
#define PAGE_TABLE_BASE         ((PageTableEntry *)0xFFC00000)

/**
 * @brief Returns whether @p addr is mapped in the current virtual address space or not
 * @param[in] addr The address to check
 */
#define ADDRESS_IS_MAPPED(addr) (IsVirtualPageMapped(aligndown(((uintptr_t)addr), PAGE_SIZE)))

/**
 * @brief Type alias for a page directory entry.
 * @details The page directory is stored as 32-bit entries on x86 with 4KiB pages.
 */
typedef volatile u32 PageDirectory;

/**
 * @brief Type alias for a page table entry.
 * @details A page table entry is a 32-bit value that describes a single mapped page.
 */
typedef volatile u32 PageTableEntry;

/**
 * @brief Initialize the virtual memory manager.
 * @details Sets up paging structures and enables virtual memory.
 * @returns 0 on success, negative value on failure.
 */
Status InitVirtualMemoryManager(void);

/**
 * @brief Load a new page directory to be used by the CPU.
 * @warning Be VERY careful when calling this function. The kernel must be already mapped within the
 * @note For performance reasons, this function uses the regpart calling convention. First parameter
 * goes directly in eax, not pushed on the stack.
 * @p addr (Page table) given. Otherwise the kernel is going to crash.
 * @param addr The PHYSICAL address of the page directory to load. Must be page aligned.
 */
[[gnu::regparm(1)]]
extern void SetPageDirectory(uintptr_t addr);

/**
 * @brief Fetch the (physical)
 * @note Before accessing the return value, it must be mapped to the virtual address space, see @ref
 * MapSinglePage
 * @return PageDirectory* The page directory currently in use
 */
static inline PageDirectory *GetCurrentPageDirectory(void) {
        u32 pdaddr = 0;
        __asm__ volatile(
                "mov %%cr3, %%eax\n"
                "mov %%eax, %0"
                : "=r"(pdaddr)
                :
                : "eax");
        return (PageDirectory *)pdaddr;
}

/**
 * @brief Convert a physical address to a kernel virtual address.
 * @details For addresses within the kernel's load region, this function maps them into
 * the kernel virtual region using the kernel base offset. For addresses at or below
 * the kernel LMA base, it maps them to a free virtual region above the kernel image.
 * @param addr Physical address to convert.
 * @returns Corresponding virtual address.
 */
static inline uintptr_t phys_to_virt(uintptr_t addr) { return addr + VIRT_OFFSET; }

/**
 * @brief Convert a kernel virtual address to a physical address.
 * @param addr Virtual address to convert.
 * @returns Corresponding physical address.
 */
static inline uintptr_t virt_to_phys(uintptr_t addr) { return addr - VIRT_OFFSET; }

/**
 * @brief Map a single physical page to a virtual address.
 * @param pd The page directory to store the new mapping to
 * @param phys Physical frame base address.
 * @param virt Virtual address to map to.
 * @param flags Page table flags (present, rw, user, etc.).
 * @returns STATUS_OK on success, STATUS_BAD on failure.
 */
Status MapSinglePage(uintptr_t phys, uintptr_t virt, u32 flags);

/**
 * @brief Unmap a single page from the virtual memory map.
 * @param pd The page directory to remove the mapping from
 * @param virt Virtual address of the page to unmap.
 */
void UnmapSinglePage(uintptr_t virt);

/**
 * @brief Returns whether the page directory currently used by the kernel has the given @p addr
 * mapped or not.
 * @param[in] addr The address to check whether it's mapped or not
 * @return true if the address is mapped, false if the address is not mapped
 */
Bool IsVirtualPageMapped(uintptr_t addr);

/**
 * @brief Map a contiguous range of physical pages to virtual memory.
 * @details Maps @p n_pages pages starting from physical address @p phys to the
 * virtual address @p virt. Both physical and virtual addresses are incremented by
 * PAGE_SIZE for each mapped page.
 * @param phys Starting physical address.
 * @param virt Starting virtual address.
 * @param flags Page table flags.
 * @param n_pages Number of pages to map.
 * @returns Number of pages successfully mapped.
 */
Size MapMemoryRange(uintptr_t phys, uintptr_t virt, u32 flags, Size n_pages);

/**
 * @brief Unmaps a series of pages starting at @p virt
 * @sa MapMemoryRange
 * @param[in] virt
 * @param[in] n_pages
 * @return STATUS_OK if all the pages were unmapped, STATUS_OUT_OF_BOUNDS if a page within that
 * range was not mapped.
 */
Status UnmapMemoryRange(uintptr_t virt, Size n_pages);

/**
 * @brief Forces the CPU to reload all virtual memory mappings and clear the translation lookaside
 * buffer.
 * @note This is a very expensive function. As such, it should only be called in a couple of
 * specific cases (like entire page tables being freed)
 */
extern void FlushTLB(void);
