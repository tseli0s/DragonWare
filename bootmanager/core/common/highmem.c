/**********************************************************************
 * FILE: highmem.c
 * PURPOSE: High memory allocation code used for modules and large structures
 * PROJECT: DragonWare Boot Manager
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "highmem.h"

#include <ktypes.h>
#include <macros.h>

#include "memdetect.h"
#include "textmode/dbgprint.h"

#define FRAME_SIZE (0x1000)

/* Where does high memory start. If any other routines use high memory without using this allocator
 * (Most notably the ELF loader), they must call AllocHighBreakAt() to update this. */
static uintptr_t himem_start = HIGH_MEMORY_START;

/* Dynamically updated by AllocHighInit(). By default, a megabyte of memory can be used in high
 * memory (which is guaranteed to be present because there are checks in the boot sector code) */
static uintptr_t himem_break = HIGH_MEMORY_START + 0x100000;

/* Number of pages left for allocation in the high memory heap. */
static Size n_pages_high = 0;

void AllocHighInit(void) {
        Size              n_regions = 0;
        MemoryRegionE820 *regions   = FetchMemoryRegions(&n_regions);
        if (unlikely(!regions)) return;

        /* We will track the largest region found */
        uintptr_t best_start = himem_start;
        uintptr_t best_break = himem_break;
        u64       max_size   = best_break - best_start;

        for (Size i = 0; i < n_regions; i++) {
                MemoryRegionE820 current = regions[i];
                if (current.type != E820_REGION_AVAILABLE) continue;
                if (current.base < HIGH_MEMORY_START) continue;
                if (unlikely(!current.length))
                        /* The fuck? Did we mess something in the E820 memory detection? */
                        continue;

                uintptr_t aligned_base = alignup(current.base, FRAME_SIZE);
                u64       endaddr      = current.base + current.length;

                if (aligned_base >= endaddr) continue;

                u64 candidate_size = endaddr - aligned_base;
                if (candidate_size > max_size) {
                        max_size   = candidate_size;
                        best_start = aligned_base;
                        best_break = endaddr;
                }
        }

        himem_start = best_start;
        himem_break = best_break;

        /*
         * Prevent low memory from being used at all in case it was selected.
         * Low memory contains the bootloader, the IVT/BDA, and other stuff that
         * we don't want to overwrite probably.
         * */
        if (himem_start < HIGH_MEMORY_START) himem_start = HIGH_MEMORY_START;

        n_pages_high = aligndown((himem_break - himem_start), FRAME_SIZE) / FRAME_SIZE;

        DebugPrint("Initialized high memory allocator (Claimed region %p-%p), claiming %d pages",
                   himem_start, himem_break, n_pages_high);
}

void AllocHighBreakAt(uintptr_t start, uintptr_t end) {
        if (start > himem_start) himem_start = start;
        if (end > 0 && end < himem_break) himem_break = end;

        if (likely(himem_break > himem_start))
                n_pages_high = aligndown((himem_break - himem_start), FRAME_SIZE) / FRAME_SIZE;
        else
                n_pages_high = 0; /* That's probably a bug somewhere in the bootloader */
}

void *AllocateHighMemory(Size n_pages) {
        if (!n_pages) return NullPointer;
        if (n_pages_high < n_pages) return NullPointer;

        void *allocated_ptr = (void *)himem_start;
        himem_start += n_pages * FRAME_SIZE;
        n_pages_high -= n_pages;

        return allocated_ptr;
}

void FreeHighMemory(void *ptr) {
        /* TODO: Implement this. Though for now, we aren't gonna use it anywhere, as
         * the primary purpose is to place modules right in high memory which are never freed. */
        UnusedParameter(ptr);

        if (!ptr) return;
}
