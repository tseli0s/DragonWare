/**********************************************************************
 * FILE: kmalloc.c
 * PURPOSE: Slab allocator for DragonWare's kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "kmalloc.h"

#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "assert.h"
#include "ddk/ia32/cpu.h"
#include "ddk/ia32/kcpuid.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "log.h"
#include "mem/frame.h"
#include "panic.h"

/******************************************************************************************
 * XXX this code is very similar to sys/task/process.c we can probably unify everything
 * in sys/ddk/ia32/vmm.c but I am a little lazy today.
 ***************************************************************************************/

/** @brief Where the heap starts in virtual memory. This assumes that the kernel is under a certain
 * size. */
#define HEAP_BASE        ((uintptr_t)KERNEL_VM_BASE + (0x100000))

/** @brief Where the kernel heap ends. */
#define HEAP_BREAK       ((uintptr_t)(HEAP_BASE + (0x20000000)))

/** @brief Size of the heap in virtual memory */
#define HEAP_SIZE        ((uintptr_t)(HEAP_BREAK - HEAP_BASE))

/** @brief Amount of bit words needed to track the heap usage of the kernel in pages. A single word
 * contains 32 bits in this case. */
#define BIT_WORDS_NEEDED ((HEAP_SIZE / PAGE_SIZE + 31) / 32)

/** @brief Calculates the bit that tracks a single virtual page in the heap bitmap and
 * returns its index. */
#define BIT_OF(addr)     ((u32)(((addr) - HEAP_BASE) / PAGE_SIZE))

static u32 heap_bitmap[BIT_WORDS_NEEDED] = {0};

static inline void MarkHeapPageAsUsed(uintptr_t addr) {
        kassert(isaligned(addr, PAGE_SIZE));
        if (!inrange(addr, HEAP_BASE, HEAP_BREAK)) return;

        u32 idx = BIT_OF(addr);
        heap_bitmap[idx / 32] |= (1 << idx % 32);
}

static inline void MarkHeapPageAsFree(uintptr_t addr) {
        kassert(isaligned(addr, PAGE_SIZE));
        if (!inrange(addr, HEAP_BASE, HEAP_BREAK)) return;

        u32 idx = BIT_OF(addr);
        heap_bitmap[idx / 32] &= ~(1 << (idx % 32));
}

static void *GetHeapPageAddress(void) {
        for (u32 i = 0; i < BIT_WORDS_NEEDED; i++) {
                if (heap_bitmap[i] == 0xFFFFFFFF) continue;

                int bit = __builtin_ctz(~heap_bitmap[i]);
                heap_bitmap[i] |= (1 << bit);

                return (void *)(HEAP_BASE + ((i * 32 + (u32)bit) * 4096));
        }
        /* I really should implement an OOM helper or something */
        FatalError("Kernel has ran out of heap memory");
}

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
	else kzeromem(ptr, PAGE_SIZE); 

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

        uintptr_t frameaddr = AllocateFrame();

        void     *virtaddr = GetHeapPageAddress();
        uintptr_t addr     = (uintptr_t)virtaddr;

        static u32 flags = PAGE_PRESENT | PAGE_RW;
        if (x86FeatureSupported(X86_PGE)) flags |= PAGE_GLOBAL;

        CheckStatus(MapSinglePage(frameaddr, addr, flags), {
                LogMessage(LOG_ERROR,
                           "MapSinglePage() did not return STATUS_OK, AllocateVirtualPage() has "
                           "nothing to return.");
                return NullPointer;
        });
        return virtaddr;
}

void FreeVirtualPage(void *addr) {
        uintptr_t vaddr = (uintptr_t)addr;
        uintptr_t paddr =
                virt_to_phys(vaddr); /* FIXME !!!!!! This only works if the offset remains constant
                                        across mappings, which we don't guarantee with absolute
                                        certainty. For now it works though.*/

        UnmapSinglePage(vaddr);
        MarkHeapPageAsFree(vaddr);
        FreeFrame(paddr);
}
