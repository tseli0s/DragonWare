/**********************************************************************
 * FILE: mm.h
 * PURPOSE: Memory probing, region handling and allocation code for the kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <vendor/multiboot.h>

/**
 * @brief Maximum number of memory regions that can be tracked.
 * @note If the bootloader provides >= MAX_MEM_REGIONS regions, they will be ignored. This is almost
 * never the case on IA32, but when the 64 bit port becomes a thing, it will need to be patched.
 */
#define MAX_MEM_REGIONS (64)

/**
 * @brief Types of physical memory regions as reported by the bootloader.
 * @details These values correspond to the Multiboot memory map entry types.
 */
typedef enum _MemoryRegionType {
        REGION_AVAILABLE        = 1, /**< Usable RAM. */
        REGION_RESERVED         = 2, /**< Reserved memory. */
        REGION_ACPI_RECLAIMABLE = 3, /**< ACPI reclaimable memory. */
        REGION_ACPI_NVS         = 4, /**< ACPI NVS memory. */
        REGION_BAD_MEMORY       = 5  /**< Bad memory that should not be used. */
} MemoryRegionType;

/**
 * @brief Represents a contiguous region of physical memory.
 * @details The structure stores whether the region is available for use, its
 * starting physical address, length in Bytes, and the region type.
 */
typedef struct _MemoryRegion {
        Bool             available; /**< True if the region is available for allocation. */
        Size             start;     /**< Starting physical address of the region. */
        Size             len;       /**< Length of the region in Bytes. */
        MemoryRegionType type;      /**< Region type (one of @ref MemoryRegionType). */
} MemoryRegion;

/**
 * @brief Populate the memory region list from Multiboot information.
 * @param mbi Pointer to the Multiboot information structure.
 * @returns STATUS_OK on success, negative value on failure.
 */
Status AddMemoryRegions(Multiboot *mbi);

/**
 * @brief Retrieve the list of parsed memory regions.
 * @param count Output parameter set to the number of regions returned.
 * @returns Pointer to an array of memory regions.
 */
MemoryRegion *FetchMemoryRegions(Size *count);
