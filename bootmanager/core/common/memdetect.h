/**********************************************************************
 * FILE: memdetect.h
 * PURPOSE: Memory detection and helpers
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#define MAX_MEMORY_REGIONS (32) /* This is hardcoded in bootmanager/bootsect/hdd/boot.asm */

/**
 * @brief An enum that describes a memory region as part of the @code type @endcode field
 * of @ref MemoryRegionE820.
 */
typedef enum _MemoryRegionType {
        E820_REGION_AVAILABLE        = 1, /**< Usable RAM. */
        E820_REGION_RESERVED         = 2, /**< Reserved memory. */
        E820_REGION_ACPI_RECLAIMABLE = 3, /**< ACPI reclaimable memory. */
        E820_REGION_ACPI_NVS         = 4, /**< ACPI NVS memory. */
        E820_REGION_BAD_MEMORY       = 5  /**< Bad memory that should not be used. */
} MemoryRegionType;

/**
 * @brief A memory region descriptor fetched from the BIOS memory discovery service int 15h,
 * eax=E820h
 * @note This is closely resembling, but not identical to the Multiboot memory regions map. Do not
 * use it as a replacement.
 */
typedef struct [[gnu::packed]] _MemoryRegionE820 {
        u64 base;
        u64 length;
        u32 type; /** << See @ref MemoryRegionType */
} MemoryRegionE820;

/* Copies the memory regions discovered by the boot sector into the second stage's preallocated
 * buffer and returns a pointer to it. Saves the amount of memory regions in n */
MemoryRegionE820 *FetchMemoryRegions(Size *n);
