/**********************************************************************
 * FILE: kmalloc.c
 * PURPOSE: Slab allocator for DragonWare's kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "kmalloc.h"

#include <mmutils.h>

#include "ddk/ia32/cpu.h"
#include "ddk/ia32/kcpuid.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "ktypes.h"
#include "log.h"
#include "macros.h"
#include "mem/frame.h"
#include "panic.h"

/** @brief Where the kernel heap ends. 0xC0000000 is the kernel virtual base. */
#define HEAP_BREAK (0x20000000)

static Size heap_allocated = 0;

/*
 * kmalloc/kfree implementation:
 *
 * After much thought I've decided to settle on a simple slab allocator.
 * Lots of examples to learn from, reliable, though the implementation did take
 * about a day to write.
 *
 * NOTE: Type, in this source file only, means "power of two". 4 is a type. 16
 * is a type. 32 is a type. And so on.
 */

/* NOTE: If I'm being honest, I have absolutely zero idea if this code works. It
 * seems to do so, but I am super suspicious of it for some reason. I had to fix
 * little things here and there so much that I'm literally not sure if it works
 * out of luck or is actually correct. */

/* Eight different slab caches, starting from 8 Bytes (2 to the power of 3, for
 * the dumdums) */
#define SLAB_TYPES_MAX    (8)

/** @brief At which boundary to align each slab object for @ref kmalloc. 16 bytes is usually a good
 * alignment for speed and cache hits. */
#define DEFAULT_OBJ_ALIGN (16)

/* For more than 1024 Bytes, we will probably allocate an entire page and return
 * that. The page allocator isn't implemented yet, I'm working on it. */
static const u32 slab_sizes[SLAB_TYPES_MAX] = {
        8, 16, 32, 64, 128, 256, 512, 1024,
};

static Bool allocator_initialized = false;

typedef struct _Slab {
        void         *free_list;
        void         *objstart;
        Size          n_objects;
        Size          objsize;
        struct _Slab *next;
} Slab;

typedef struct _SlabCache {
        Slab *slabs;
        Size  objsize;
} SlabCache;

static SlabCache caches[SLAB_TYPES_MAX];

static inline void InitSlabCaches(void) {
        for (int i = 0; i < SLAB_TYPES_MAX; i++) {
                caches[i].objsize = slab_sizes[i];
                caches[i].slabs   = NullPointer;
        }
}

static inline SlabCache *GetSlabCacheForSize(Size size) {
        for (int i = 0; i < SLAB_TYPES_MAX; i++)
                if (size <= slab_sizes[i]) return &caches[i];
        return NullPointer; /* Too big, we have to allocate a page instead and skip the
                        slabs. */
}

static Slab *CreateSlab(Size objsize) {
        if (objsize < sizeof(void *)) objsize = sizeof(void *);

        void *frame = AllocateVirtualPage();
        Slab *slab  = (Slab *)frame;
        if (!slab) return NullPointer;

        slab->objsize = objsize;
        slab->next    = NullPointer;

        uintptr_t base    = (uintptr_t)slab + sizeof(Slab);
        uintptr_t aligned = alignup(base, DEFAULT_OBJ_ALIGN);
        slab->objstart    = (void *)aligned;

        Size usable     = PAGE_SIZE - (aligned - (uintptr_t)slab);
        slab->n_objects = usable / objsize;

        if (slab->n_objects == 0) {
                FreeVirtualPage(frame);
                return NullPointer;
        }

        char *ptr       = (char *)slab->objstart;
        slab->free_list = ptr;

        for (Size i = 0; i < slab->n_objects - 1; i++) {
                *(void **)ptr = ptr + objsize;
                ptr += objsize;
        }
        *(void **)ptr = NullPointer;
        return slab;
}

void *kmalloc(Size size) {
        if (size == 0) return NullPointer;
        /* inrange() is inclusive, so we must add 1 to the lower boundary or it may take a normal
         * slab allocation and think we are trying to allocate a virtual page instead */
        else if (inrange(size, slab_sizes[SLAB_TYPES_MAX - 1] + 1, PAGE_SIZE))
                return AllocateVirtualPage();
        else if (size > PAGE_SIZE)
                return NullPointer; /* Huge allocation, not supported yet */

        if (!unlikely(allocator_initialized)) {
                InitSlabCaches();
                allocator_initialized = true;
        }

        SlabCache *cache = GetSlabCacheForSize(size);
        Slab      *s     = cache->slabs;
        while (s && !s->free_list) s = s->next;

        if (!s) {
                s = CreateSlab(cache->objsize);
                if (!s) {
                        LogMessage(LOG_ERROR, "Unable to allocate internal slab cache");
                        goto nomem;
                }

                s->next      = cache->slabs;
                cache->slabs = s;
        }

        void *obj    = s->free_list;
        s->free_list = *(void **)obj;

        return obj;

nomem:
        return NullPointer;
}

void *kzalloc(Size size) {
        void *ptr = kmalloc(size);
        if (!ptr) return NullPointer;

        /* We need to zero out the entire slab, not just the size requested. Otherwise previous
         * (larger) allocations will leak data. */
        SlabCache *cache = GetSlabCacheForSize(size);
        /* If this never runs, we've allocated a page or more. */
        if (cache) kzeromem(ptr, cache->objsize);

        return ptr;
}

void kfree(void *ptr) {
        if (!ptr) return;
        for (int i = 0; i < SLAB_TYPES_MAX; i++) {
                SlabCache *cache = &caches[i];
                Slab      *s     = cache->slabs;

                while (s) {
                        char *start = (char *)s->objstart;
                        char *end   = start + s->n_objects * s->objsize;

                        if ((char *)ptr >= start && (char *)ptr < end) {
                                /* Unaligned pointer check */
                                if (((uintptr_t)ptr - (uintptr_t)start) % s->objsize != 0) {
                                        LogMessage(LOG_ERROR,
                                                   "kfree: pointer %p is not aligned to slab "
                                                   "object size",
                                                   ptr);
                                        return;
                                }
                                /* Found this cool idea where you store the pointer to the next free
                                 * block using the uninitialized memory. Don't remember where, I
                                 * think it was Linux. */
                                *(void **)ptr = s->free_list;
                                s->free_list  = ptr;
                                return;
                        }
                        s = s->next;
                }
        }
        /* If we're here, we are probably trying to free a frame. If that didn't work either, just
         * move on. */
        FreeVirtualPage(ptr);
}

void *AllocateVirtualPage(void) {
        extern char _end;
        if (unlikely(heap_allocated >= HEAP_BREAK))
                FatalError("No virtual memory left for the kernel's heap!");

        uintptr_t frameaddr = AllocateFrame();
        uintptr_t heap_next = pagealign((uintptr_t)&_end + heap_allocated);
        heap_allocated += PAGE_SIZE;

        static u32 flags = PAGE_PRESENT | PAGE_RW;
        if (x86FeatureSupported(X86_PGE)) flags |= PAGE_GLOBAL;

        CheckStatus(MapSinglePage(frameaddr, heap_next, flags), {
                LogMessage(LOG_ERROR,
                           "MapSinglePage() did not return STATUS_OK, AllocateVirtualPage() has "
                           "nothing to return.");
                return NullPointer;
        });
        return (void *)heap_next;
}

void FreeVirtualPage(void *addr) {
        uintptr_t vaddr = (uintptr_t)addr;
        uintptr_t paddr =
                virt_to_phys(vaddr); /* FIXME !!!!!! This only works if the offset remains constant
                                        across mappings, which we don't guarantee with absolute
                                        certainty. For now it works though.*/

        UnmapSinglePage(vaddr);
        FreeFrame(paddr);
}
