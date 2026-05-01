/**********************************************************************
 * FILE: mm.c
 * PURPOSE: Memory probing, region handling and allocation code for the kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "mm.h"

#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>
#include <panic.h>
#include <vendor/multiboot.h>

#include "ddk/ia32/vmm.h" /* KERNEL_VM_BASE, KERNEL_LMA_BASE */

static MemoryRegion mem_regions[MAX_MEM_REGIONS] = {0};
static Size         mem_regions_idx              = 0;

extern uintptr_t _begin;

static inline Size kernel_phys_start(void) {
        return (Size)((uintptr_t)&_begin - KERNEL_VM_BASE + KERNEL_LMA_BASE);
}

static inline Size kernel_phys_end(void) {
        extern char _end;
        return (Size)((uintptr_t)&_end - KERNEL_VM_BASE + KERNEL_LMA_BASE);
}

/**
 * @brief A region within available memory that is reserved for other purposes (eg. Kernel,
 * bootloader structures, MMIO, ...)
 */
typedef struct _ReservedRegion {
        uintptr_t start;
        uintptr_t end;
} ReservedRegion;

/**
 * @brief Helper function to sort an array of reserved regions from lowest memory address to highest
 * memory address.
 * @param[in] regions Pointer to the array of ReservedChunks
 * @param[in] n Amount of regions in the array
 */
[[gnu::nonnull(1)]]
static void SortReservedChunks(ReservedRegion *regions, Size n) {
        for (Size i = 1; i < n; i++) {
                ReservedRegion key = regions[i];
                Size           j   = i - 1;
                while (j >= 0 && regions[j].start > key.start) {
                        regions[j + 1] = regions[j];
                        j--;
                }
                regions[j + 1] = key;
        }
}
Status AddMemoryRegions(Multiboot *bootinfo) {
        if (!bootinfo || !bootinfo->mmap_addr)
                FatalError("Bootloader did not provide memory region information");

        kzeromem(mem_regions, sizeof(mem_regions));
        mem_regions_idx = 0;

        ReservedRegion reserved[64]; /* Uh guys I hope 64 is enough */
        Size           reserved_count = 0;

        reserved[reserved_count].start = aligndown(kernel_phys_start(), PAGE_SIZE);
        reserved[reserved_count].end   = pagealign(kernel_phys_end());
        reserved_count++;

        if (bootinfo->flags & MULTIBOOT_MODS && bootinfo->mods_count > 0) {
                MultibootModule *mods = (MultibootModule *)bootinfo->mods_addr;
                for (unsigned int j = 0; j < bootinfo->mods_count; j++) {
                        if (reserved_count >= arraysize(reserved)) {
                                LogMessage(LOG_WARNING,
                                           "Max reserved chunks reached, some modules might be "
                                           "overwritten by kernel data!");
                                break;
                        }
                        reserved[reserved_count].start = aligndown(mods[j].start, PAGE_SIZE);
                        reserved[reserved_count].end   = pagealign(mods[j].end);
                        reserved_count++;
                }
        }

        SortReservedChunks(reserved, reserved_count);

        uintptr_t idx     = (uintptr_t)bootinfo->mmap_addr;
        uintptr_t endaddr = idx + bootinfo->mmap_len;

        while (idx < endaddr) {
                volatile MultibootMMapEntry *current = (volatile MultibootMMapEntry *)idx;

                u64 start = (u64)current->addr;
                u64 end   = (u64)(((u64)current->addr + (u64)current->len) - 1);

                switch (current->type) {
                        case REGION_AVAILABLE:
                                LogMessage(LOG_DEBUG, "Memory region %r-%r is available", start,
                                           end);
                                break;
                        case REGION_RESERVED:
                                LogMessage(LOG_DEBUG, "Memory region %r-%r is reserved", start,
                                           end);
                                break;
                        case REGION_BAD_MEMORY:
                                LogMessage(LOG_ERROR, "Memory addresses %r-%r are BAD MEMORY!",
                                           start, end);
                                break;
                        case REGION_ACPI_NVS:
                                LogMessage(LOG_DEBUG, "Memory region %r-%r is ACPI NVS", start,
                                           end);
                                break;
                        case REGION_ACPI_RECLAIMABLE:
                                LogMessage(LOG_DEBUG, "Memory region %r-%r is ACPI reclaimable",
                                           start, end);
                                break;
                        default:
                                LogMessage(LOG_WARNING, "Unknown memory region type: %d",
                                           current->type);
                                break;
                }

                if (current->type == REGION_AVAILABLE || current->type == REGION_ACPI_RECLAIMABLE) {
                        Size region_start = (Size)current->addr;
                        Size region_end   = region_start + (Size)current->len;
                        Size curr_start   = region_start;

                        for (Size i = 0; i < reserved_count; i++) {
                                if (reserved[i].end <= curr_start) continue;
                                if (reserved[i].start >= region_end) break;
                                if (curr_start < reserved[i].start) {
                                        MemoryRegion mm = {.available = true,
                                                           .type      = current->type,
                                                           .start     = curr_start,
                                                           .len = reserved[i].start - curr_start};
                                        if (mem_regions_idx < MAX_MEM_REGIONS)
                                                mem_regions[mem_regions_idx++] = mm;
                                }
                                if (reserved[i].end > curr_start) curr_start = reserved[i].end;
                        }
                        if (curr_start < region_end) {
                                MemoryRegion mm = {.available = true,
                                                   .type      = current->type,
                                                   .start     = curr_start,
                                                   .len       = region_end - curr_start};
                                if (mem_regions_idx < MAX_MEM_REGIONS)
                                        mem_regions[mem_regions_idx++] = mm;
                        }
                }

                idx += current->size + sizeof(current->size);
                if (mem_regions_idx >= MAX_MEM_REGIONS) break;
        }

        Size total_mem_bytes = 0;
        for (Size i = 0; i < mem_regions_idx; i++) total_mem_bytes += mem_regions[i].len;

        Size kstart = aligndown(kernel_phys_start(), PAGE_SIZE);
        Size kend   = pagealign(kernel_phys_end());

        LogMessage(LOG_INFO,
                   "Detected %d KBs of memory on host, %d memory regions marked available",
                   (total_mem_bytes / 1024) - ((kend - kstart) / 1024), mem_regions_idx);

        return STATUS_OK;
}

MemoryRegion *FetchMemoryRegions(Size *count) {
        *count = mem_regions_idx;
        return mem_regions;
}
