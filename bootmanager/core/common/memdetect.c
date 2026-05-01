/**********************************************************************
 * FILE: bootentry.c
 * PURPOSE: Memory detection and sorting implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "memdetect.h"

#include <mmutils.h>
#include <stdbool.h>

#include "textmode/dbgprint.h"

static MemoryRegionE820 memory_regions[MAX_MEMORY_REGIONS] = {0};
static Size             n_memory_regions                   = 0;
static bool             fetched_already                    = false;

static void BubbleSortMemoryRegions(void) {
        for (Size i = 0; i < n_memory_regions - 1; i++) {
                bool swapped = false;
                for (Size j = 0; j < n_memory_regions - i - 1; j++) {
                        if (memory_regions[j].base > memory_regions[j + 1].base) {
                                MemoryRegionE820 tmp  = memory_regions[j];
                                memory_regions[j]     = memory_regions[j + 1];
                                memory_regions[j + 1] = tmp;
                                swapped               = true;
                        }
                }
                if (!swapped) break;
        }
}
MemoryRegionE820 *FetchMemoryRegions(Size *n) {
        /* Provided by loader.asm when the second stage is loaded */
        extern Word              NumMemoryRegions;
        extern MemoryRegionE820 *MemoryRegionsList;

        if (!fetched_already) {
                *n               = NumMemoryRegions;
                n_memory_regions = NumMemoryRegions;

                memcpy((void *)memory_regions, &MemoryRegionsList,
                       sizeof(MemoryRegionE820) * n_memory_regions);
                /* There is no guarantee that the memory regions are going to be sorted by their
                 * start address. While not required, it's probably best if we sort it before
                 * passing it to the kernel */
                BubbleSortMemoryRegions();

                fetched_already = true;

                for (unsigned int i = 0; i < n_memory_regions; i++) {
                        const MemoryRegionE820 curr = memory_regions[i];
                        DebugPrint("Memory region %d: Type %d, start 0x%x, size 0x%x", i, curr.type,
                                   (u32)curr.base, (u32)curr.length);
                }
        }
        if (n) *n = n_memory_regions;
        return memory_regions;
}
