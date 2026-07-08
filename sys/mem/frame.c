/**********************************************************************
 * FILE: frame.c
 * PURPOSE: Bitmap-based kernel physical memory management
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/*
 * This memory manager was rewritten entirely from scratch in July 2026. It switched to a bitmap
 * based allocator, removed some weird quirks the previous implementation had related to freeing,
 * and focused on improved performance and low allocation overhead. I wanted to leave this note here
 * so that other people can read about the changelog towards v0.0.2.
 *
 * By design, we ignore the first two frames to be sure that we never touch the BIOS/bootloader
 * stuff. This avoids bugs where pointers pointing to 0 are writeable and
 * allocatable. (Of course they're not supposed to be writeable or allocatable!). This also means
 * any physical address below 0x2000 cannot be considered a valid address (That also includes
 * NullPointer). It's something that holds from the earliest days of DragonWare (actually, it didn't
 * even have a name at that point), and much of the codebase assumes this to be the case.
 */

#include "frame.h"

#include <early_kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "lib/assert.h"
#include "mm.h"
#include "panic.h"

/* Four gigabytes of physical memory, divided by the size of a single frame, that's 4294967296 /
 * 4096 = 1048576, divided by 32 bits per dword, that's 32768. Alignment because I don't know I like
 * it when things are aligned.
 *
 * Currently, yes, it's hardcoded to be always 4GBs at most, as we don't support PAE. When we move
 * to 64 bit processor support, this will be configurable, obviously, and I'm thinking of a hard
 * limit of 64GBs and using another allocator if somebody has more RAM than that.
 *
 */
[[gnu::aligned(sizeof(DoubleWord))]]
static u32 bitmap[32768] = {0};

/*
 * This tracks the highest available page aligned address reported available by the
 * bootloader/firmware. It's an easy way to catch bad calls to FreeFrame() without having to parse
 * the memory map every time, though it doesn't protect against other ways.
 */
static uintptr_t highest_addr = 0;

static inline void MarkFrameAvailable(uintptr_t frameaddr) {
        /* Skip the first two frames to be sure that we never touch the BIOS/bootloader
         * stuff. This avoids bugs where pointers pointing to 0 are writeable and
         * allocatable. (Of course they're not supposed to be writeable or allocatable!)
         */
        if (unlikely(frameaddr < 2 * FRAME_SIZE)) return;
        if (frameaddr > highest_addr) return;

        u32 index = (frameaddr / FRAME_SIZE) / 32;
        u32 bit   = (frameaddr / FRAME_SIZE) % 32;
        bitmap[index] &= ~(1U << bit);
}

static inline void MarkFrameReserved(uintptr_t frameaddr) {
        u32 index = (frameaddr / FRAME_SIZE) / 32;
        u32 bit   = (frameaddr / FRAME_SIZE) % 32;
        bitmap[index] |= (1U << bit);
}

static inline Bool IsFrameAllocated(uintptr_t frameaddr) {
        u32 index = (frameaddr / FRAME_SIZE) / 32;
        u32 bit   = (frameaddr / FRAME_SIZE) % 32;
        return (bitmap[index] & (1U << bit));
}

static uintptr_t GetFirstFreeHighFrame(void) {
        /* (1048576 / 4096) / 32 = 8 = First high memory address available in the bitmap */
        for (u32 i = 8; i < arraysize(bitmap); i++) {
                if (bitmap[i] == 0xFFFFFFFF) continue;

                int bit = __builtin_ctz(~bitmap[i]);
                return (i * 32 + (u32)bit) * FRAME_SIZE;
        }
        return 0;
}

static uintptr_t GetFirstFreeLowFrame(void) {
        /* (1048576 / 4096) / 32 = 8 = Valid bitmap low memory */
        for (u32 i = 0; i < 8; i++) {
                if (bitmap[i] == 0xFFFFFFFF) continue;

                int bit = __builtin_ctz(~bitmap[i]);
                return (i * 32 + (u32)bit) * FRAME_SIZE;
        }
        return 0;
}

void InitFrameManager(void) {
        memset(bitmap, 0xFF, sizeof(bitmap));

        Size          n_regions = 0;
        MemoryRegion *regions   = FetchMemoryRegions(&n_regions);

        if (n_regions < 1) {
                FatalError(
                        "There are no usable memory regions in this machine! "
                        "This is possibly a bug, please report it at "
                        "https://github.com/tseli0s/DragonWare/issues");
        }

        for (Size i = 0; i < n_regions; i++) {
                MemoryRegion current = regions[i];
                if (!current.available) continue;

                Size end = current.start + current.len;
                if (end > highest_addr) highest_addr = end;

                /* Generally addresses are page aligned. I haven't seen anything other than that. If
                 * I do, I'll update this code, until then I think this assumption helps keeping
                 * things clean. */
                for (Size j = current.start; j < current.start + current.len; j += FRAME_SIZE)
                        MarkFrameAvailable(j);
        }
}

[[gnu::hot]]
uintptr_t AllocateFrame(void) {
        uintptr_t frame = GetFirstFreeHighFrame();
        MarkFrameReserved(frame);
        return frame;
}

uintptr_t AllocateLowMemory(void) {
        uintptr_t frame = GetFirstFreeLowFrame();
        MarkFrameReserved(frame);
        return frame;
}

void FreeFrame(uintptr_t frameaddr) {
/* Just for performance reasons, we'll only check this in debug builds */
#ifdef DRAGONWARE_DEBUG_MODE
        if (frameaddr < 2 * FRAME_SIZE) {
                LogMessage(LOG_DEBUG,
                           "Address %p too low to be freed, ignoring call to FreeFrame()",
                           frameaddr);
                return;
        }
#endif /* DRAGONWARE_DEBUG_MODE */

        /* Make sure we're trying to actually free a frame and not garbage */
        kassert(isaligned(frameaddr, FRAME_SIZE));
        if (!IsFrameAllocated(frameaddr)) {
#ifdef DRAGONWARE_DEBUG_MODE
                LogMessage(LOG_ERROR, "Attempting to double free a frame at address %p", frameaddr);
#endif
                return;
        }
        MarkFrameAvailable(frameaddr);
}
