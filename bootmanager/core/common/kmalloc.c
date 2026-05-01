/**********************************************************************
 * FILE: kmalloc.c
 * PURPOSE: Slab allocator implementation for heap allocation purposes
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "kmalloc.h"

#include <ktypes.h>
#include <mmutils.h>

#include "frame.h"

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

/* Eight different slab caches, starting from 8 Bytes (2 to the power of 3) up to 1024 (2 to the
 * power of 8) */
#define SLAB_TYPES_MAX 8

/* For more than 1024 Bytes, we will probably allocate an entire page and return
 * that. The page allocator isn't implemented yet, I'm working on it. */
static const u32 slab_sizes[SLAB_TYPES_MAX] = {
        8, 16, 32, 64, 128, 256, 512, 1024,
};

static bool allocator_initialized = false;

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

static inline void init_slab_caches(void) {
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

        void *frame = (void *)AllocateFrame();
        Slab *slab  = (Slab *)frame;
        if (!slab) return NullPointer;

        slab->objsize = objsize;
        slab->next    = NullPointer;

        uintptr_t base    = (uintptr_t)slab + sizeof(Slab);
        uintptr_t aligned = (base + 0x7) & ~((uintptr_t)0x7);
        slab->objstart    = (void *)aligned;

        Size usable     = PAGE_SIZE - (aligned - (uintptr_t)slab);
        slab->n_objects = usable / objsize;

        if (slab->n_objects == 0) {
                FreeFrame((uintptr_t)frame);
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
        if (size > slab_sizes[SLAB_TYPES_MAX - 1] && size <= FRAME_SIZE)
                return (void *)AllocateFrame();

        if (!allocator_initialized) {
                init_slab_caches();
                allocator_initialized = true;
        }

        SlabCache *cache = GetSlabCacheForSize(size);
        Slab      *s     = cache->slabs;
        while (s && !s->free_list) s = s->next;

        if (!s) {
                s = CreateSlab(cache->objsize);
                if (!s) return NullPointer;

                s->next      = cache->slabs;
                cache->slabs = s;
        }

        void *obj    = s->free_list;
        s->free_list = *(void **)obj;
        return obj;
}

void *kzalloc(Size size) {
        void *ptr = kmalloc(size);
        if (!ptr) return NullPointer;
        if (size > 1024) {
                memset(ptr, 0, PAGE_SIZE);
                return ptr;
        }

        /* We need to zero out the entire slab, not just the size requested. Otherwise previous
         * (larger) allocations will leak data. */
        SlabCache *cache = GetSlabCacheForSize(size);
        if (cache)
                kzeromem(ptr, cache->objsize);
        else
                /* Frame sized allocations aren't registered as slabs, we assume that behaviour. */
                kzeromem(ptr, FRAME_SIZE);

        return ptr;
}

void kfree(void *ptr) {
        if (!ptr || ((uintptr_t)ptr < PAGE_SIZE)) return;
        for (int i = 0; i < SLAB_TYPES_MAX; i++) {
                SlabCache *cache = &caches[i];
                Slab      *s     = cache->slabs;

                while (s) {
                        char *start = (char *)s->objstart;
                        char *end   = start + s->n_objects * s->objsize;

                        if ((char *)ptr >= start && (char *)ptr < end) {
                                if (((uintptr_t)ptr - (uintptr_t)start) % s->objsize != 0) {
                                        /* Not aligned */
                                        return;
                                }
                                *(void **)ptr = s->free_list;
                                s->free_list  = ptr;
                                return;
                        }
                        s = s->next;
                }
        }
        /* Pointer doesn't belong to any slab, see if we can free a frame instead. */
        FreeFrame((uintptr_t)ptr);
}
