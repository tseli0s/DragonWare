/**********************************************************************
 * FILE: frame.c
 * PURPOSE: Simple frame allocator implementation
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "frame.h"

#include <early_kmalloc.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "lib/assert.h"
#include "log.h"
#include "mm.h"
#include "panic.h"

#define MAX_POOLS_PHYS                                                      \
        (MAX_MEM_REGIONS - 1) /* I mean, at least one memory region will be \
                                reserved right? */
extern const char _begin;
extern const char _end;

typedef struct _PhysicalPool {
        uintptr_t frame_start; /* mem_start / FRAME_SIZE */
        Size      n_frames;    /* mem_len / FRAME_SIZE */
} PhysicalPool;

typedef struct _FreeRange {
        uintptr_t          frame_start;
        Size               n_frames;
        struct _FreeRange *next;
} FreeRange;

static PhysicalPool pool[MAX_POOLS_PHYS];
static FreeRange   *list      = NullPointer;
static Size         pool_size = 0;

static inline Size GetListSize(void) {
        Size       s    = 0;
        FreeRange *iter = list;
        while (iter != NullPointer) {
                iter = iter->next;
                s++;
        }
        return s;
}

static void AddPoolRegion(uintptr_t start, uintptr_t len) {
        pool[pool_size].frame_start = start / FRAME_SIZE;
        pool[pool_size].n_frames    = len / FRAME_SIZE;

        FreeRange *node   = AllocateStaticMemory(sizeof(FreeRange));
        node->frame_start = pool[pool_size].frame_start;
        node->n_frames    = pool[pool_size].n_frames;
        node->next        = NullPointer;

        if (!list) {
                list = node;
        } else {
                FreeRange *iter = list;
                while (iter->next) iter = iter->next;
                iter->next = node;
        }

        if ((pool_size + 1) >= MAX_POOLS_PHYS)
                LogMessage(LOG_WARNING,
                           "Too many PMM pools! Won't continue registering more pools.");
        else
                pool_size++;
}

void InitFrameManager(void) {
        kzeromem(pool, sizeof(pool));

        Size          n_regions = 0;
        MemoryRegion *regions   = FetchMemoryRegions(&n_regions);

        for (Size i = 0; i < n_regions; i++) {
                MemoryRegion current = regions[i];
                if (!current.available) continue;

                /* Skip the first two frames to be sure that we never touch the BIOS/bootloader
                 * stuff. This avoids bugs where pointers pointing to 0 are writeable and
                 * allocatable. (Of course they're not supposed to be writeable or allocatable!)
                 */
                if (current.start == 0x0) {
                        current.start += 2 * FRAME_SIZE;
                        current.len -= 2 * FRAME_SIZE;
                }

                AddPoolRegion(current.start, current.len);
        }
        if (GetListSize() < 1) {
                FatalError(
                        "There are no usable memory regions in this machine! "
                        "This is possibly a bug, please report it at "
                        "https://github.com/tseli0s/DragonWare/issues");
        }
}

[[gnu::hot]]
uintptr_t AllocateFrame(void) {
        FreeRange *current = list;
        uintptr_t  ptr     = 0x0;

        while (current) {
                if (current->n_frames > 0 && (current->frame_start * FRAME_SIZE) >= 0x00100000) {
                        ptr = current->frame_start * FRAME_SIZE;

                        current->frame_start++;
                        current->n_frames--;
                        return ptr;
                }
                current = current->next;
        }

        if (!ptr) {
                LogMessage(LOG_WARNING,
                           "Machine is running out of memory, forced to allocate from low memory.");

                return AllocateLowMemory();
        }

        return ptr;
}

uintptr_t AllocateLowMemory(void) {
        FreeRange *current = list;
        while (current) {
                if (current->n_frames > 0 && (current->frame_start * FRAME_SIZE) <= 0x00100000) {
                        uintptr_t ptr = current->frame_start * FRAME_SIZE;

                        current->frame_start++;
                        current->n_frames--;

                        return ptr;
                }
                current = current->next;
        }
        FatalError("Host ran out of memory!");
}

void FreeFrame(uintptr_t frameaddr) {
        /* Make sure we're trying to actually free a frame and not garbage */
        kassert(isaligned(frameaddr, FRAME_SIZE));
        uintptr_t frame_index = frameaddr / FRAME_SIZE;

        FreeRange *prev    = NullPointer;
        FreeRange *current = list;

        while (current && current->frame_start < frame_index) {
                prev    = current;
                current = current->next;
        }

        if (prev && (prev->frame_start + prev->n_frames == frame_index)) {
                prev->n_frames++;
                if (current && frame_index + 1 == current->frame_start) {
                        prev->n_frames += current->n_frames;
                        prev->next = current->next;
                }
                return;
        }

        if (current && frame_index + 1 == current->frame_start) {
                current->frame_start--;
                current->n_frames++;
                return;
        }

        FreeRange *node   = AllocateStaticMemory(sizeof(FreeRange));
        node->frame_start = frame_index;
        node->n_frames    = 1;
        node->next        = current;

        if (prev)
                prev->next = node;
        else
                list = node;
}
