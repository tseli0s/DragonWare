/**********************************************************************
 * FILE: process.c
 * PURPOSE: Process implementation for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "process.h"

#include <atomic.h>
#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>
#include <panic.h>

#ifdef __i386__
#include "ddk/ia32/irq.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#endif /* __i386__ */
#include "iomgr/object.h"
#include "mem/frame.h"
#include "sched/schedule.h"
#include "task/task.h"

/**
 * All kernel stacks allocated for all processes are mapped to virtual address range
 * 0xE0000000-0xEFFF0000. If there's not enough virtual memory left to map a new stack, the kernel
 * is simply going to panic. That's about 65000 pages, times 4KBs each, that's 256MBs of a maximum
 * kernel stack size. The kernel uses an 8KB stack by default.
 * XXX this code could use some cleanups for sure, there's a lot going on here to support temporary
 * virtual maps, stack allocations and so on. Feel free to take a crack at it if you want.
 */

/* ---------------------------------------------------------------------------------------------*/

/* Where do flat binaries expect execution to start (In other words, where their origin is set) */
#define FLAT_BINARY_DEFAULT_ENTRY ((ThreadEntryPoint)(0x100000))

/* Size of the stack in pages. Each page = 4096 bytes, so 16KBs of stack per process. */
#define USER_STACK_SIZE_PAGES     (4)

/* Max pages that may be allocated temporarily starting at 0xEFE00000 */
#define TMPMAP_MAX_PAGES          (1024)
#define TMPMAP_BITMAP_WORDS       (TMPMAP_MAX_PAGES / 32)

/* Random virtual address to use for our virtual mappings */
#define TEMPORARY_VIRT_MAPADDR    (0xF2000000UL)

static u32 tmpmap_bitmap[TMPMAP_BITMAP_WORDS] = {0};

/* Contains a single virtual->physical translation, used for temporary virtual address mappings. */
typedef struct _VirtualMap {
        VMAddress           addr;
        Size                slot;
        struct _VirtualMap *next;
} VirtualMap;

static ProcessID process_id_counter = 1;
static Process  *process_list       = NullPointer;

[[gnu::pure]]
static int FindFreeTmpMapSlot(void) {
        for (Size w = 0; w < TMPMAP_BITMAP_WORDS; ++w) {
                if (tmpmap_bitmap[w] != 0xFFFFFFFF) {
                        for (unsigned int b = 0; b < 32; ++b) {
                                if (!(tmpmap_bitmap[w] & (1u << b))) return (int)((w * 32) + b);
                        }
                }
        }
        return -1;
}

static inline void MarkTmpMapUsed(Size idx) {
        if (idx >= TMPMAP_MAX_PAGES) FatalError("index out of range");
        tmpmap_bitmap[idx / 32] |= (1u << (idx % 32));
}

static inline void MarkTmpMapFree(Size idx) {
        if (idx >= TMPMAP_MAX_PAGES) FatalError("index out of range");
        tmpmap_bitmap[idx / 32] &= ~(1u << (idx % 32));
}

[[gnu::pure]]
static inline int TmpMapUsed(Size idx) {
        if (idx >= TMPMAP_MAX_PAGES) FatalError("index out of range");
        return (tmpmap_bitmap[idx / 32] & (1u << (idx % 32))) != 0;
}

[[gnu::const]]
static inline uintptr_t TmpMapAddrFromIndex(Size idx) {
        return TEMPORARY_VIRT_MAPADDR - (idx * PAGE_SIZE);
}

/**
 * @brief Create a single virtual->physical temporary mapping and map it to the current address
 * space to access the allocated page from the virtual address given, similar to @ref kmalloc but
 * allows the underlying page table to be accessed directly.
 * @return The temporary virtual-physical map created. The memory at the virtual address given is
 * mapped to the current address space and may be used immediately.
 */
static VirtualMap *CreateTempVirtualMap(void) {
        VirtualMap *m = kmalloc(sizeof(VirtualMap));
        if (!m) return NullPointer;

        int slot = FindFreeTmpMapSlot();
        if (slot < 0) {
                kfree(m);
                return NullPointer;
        }

        uintptr_t virt = TmpMapAddrFromIndex((Size)slot);

        u32 phys = AllocateFrame();
        if (!phys) {
                kfree(m);
                return NullPointer;
        }

        if (MapSinglePage(phys, virt, PAGE_PRESENT | PAGE_RW) != STATUS_OK) {
                FreeFrame(phys);
                kfree(m);
                return NullPointer;
        }

        MarkTmpMapUsed((Size)slot);

        m->addr.phys = phys;
        m->addr.virt = virt;
        m->slot      = (Size)slot;

        kzeromem((void *)m->addr.virt, PAGE_SIZE);
        return m;
}

/**
 * @brief Deletes a temporarily mapped virtual address (eg. Used to access a temporary page table)
 * @param m The memory map to free. Must be nonnull.
 * @warning The actual frame allocated for the temporary virtual page is not freed,
 * FreeFrame(m->phys) must be called to avoid memory leaks (eg. in error handling paths)
 */
[[gnu::nonnull(1)]]
static void DeleteTempVirtualMap(VirtualMap *m) {
        if (!TmpMapUsed(m->slot)) FatalError("Double free in temporary mapping allocator");

        UnmapSinglePage(m->addr.virt);
        MarkTmpMapFree(m->slot);
        kfree(m);
}

/* Returns a virtual address that is free to map the kernel stack for a process contiguously. Panics
 * if it can't find one. */
static uintptr_t GetNextKernelStackAddress(void) {
        uintptr_t current_addr = KERNEL_STACK_BASE;

        /* the kernel stack pages must be continuous, hence why we are checking all pages that we
         * are planning to map as the kernel stack */
        while (current_addr + (KERNEL_STACK_SIZE_PAGES * PAGE_SIZE) <= KERNEL_STACK_END) {
                Bool conflict = false;
                for (unsigned int i = 0; i < KERNEL_STACK_SIZE_PAGES && !conflict; i++) {
                        VirtualAddress this = current_addr + (i * PAGE_SIZE);
                        if (IsVirtualPageMapped(this)) conflict = true;
                }
                if (!conflict)
                        return current_addr;
                else
                        current_addr += KERNEL_STACK_SIZE_PAGES * PAGE_SIZE;
        }

        FatalError("Not enough virtual memory left for the kernel stacks.");
}

Process *CreateProcess(ProcessID pid, void *code, Size code_size) {
        UnusedParameter(pid);
        if (code_size == 0) {
                LogMessage(LOG_ERROR, "Attempted to copy a 0-byte executable!");
                return NullPointer;
        }
        /* Allocate a kernel stack for the process. Each kernel stack is two frames large. We need a
         * separate stack per process, both in physical and virtual memory.
         * Notice how we do this before we copy the kernel into the other process' address space, so
         * the mapping persists across all processes and we can just switch the address space at any
         * point.
         */
        uintptr_t kernel_stack_addr = GetNextKernelStackAddress();
        VMAddress kstacks[KERNEL_STACK_SIZE_PAGES];
        Process  *p = kzalloc(sizeof(Process));
        if (!p) return NullPointer;

        for (unsigned int i = 0; i < KERNEL_STACK_SIZE_PAGES; i++) {
                kstacks[i] = (VMAddress){
                        .phys = AllocateFrame(),
                        .virt = kernel_stack_addr + (i * PAGE_SIZE),
                };
                MapSinglePage(kstacks[i].phys, kstacks[i].virt, PAGE_PRESENT | PAGE_RW);
                p->kstacks[i] = kstacks[i];
        }

        Thread *main_thread = AllocateUserThread(FLAT_BINARY_DEFAULT_ENTRY, DEFAULT_USER_STACK_ADDR,
                                                 kernel_stack_addr + (2 * FRAME_SIZE), NullPointer);
        if (!main_thread) return NullPointer;

        /* First, allocate the process' page directory. */
        VirtualMap *pdmap = CreateTempVirtualMap();
        if (!pdmap) {
                kfree(p);
                return NullPointer;
        }

        /* It's very important that we shallow copy the kernel space otherwise any change in any
         * state within the kernel will not reflect on the other processes.
         * The last PD entry is reserved for the page directory (Recursive paging trick), which is
         * why it is not copied.
         */
        for (int i = KERNEL_PD_INDEX; i < MAX_PD_ENTRIES - 1; i++)
                ((PageDirectory *)pdmap->addr.virt)[i] = CURRENT_PAGE_DIRECTORY[i];

        /* Now allocate the page table for the process and map it in the address space. */
        VirtualMap *ptmap = CreateTempVirtualMap();
        kzeromem((void *)ptmap->addr.virt, PAGE_SIZE);

        /* Allocate a user stack while we're at it */
        VirtualMap *stackmap = CreateTempVirtualMap();
        kzeromem((void *)stackmap->addr.virt, PAGE_SIZE);
        for (unsigned int i = 0; i < USER_STACK_SIZE_PAGES; i++) {
                const Size  offset    = i * PAGE_SIZE;
                const Size  ptindex   = PT_INDEX(DEFAULT_USER_STACK_ADDR - offset);
                VirtualMap *currstack = CreateTempVirtualMap();
                ((PageTableEntry *)stackmap->addr.virt)[ptindex] =
                        currstack->addr.phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
                DeleteTempVirtualMap(currstack);
        }
        ((PageDirectory *)pdmap->addr.virt)[PD_INDEX(DEFAULT_USER_STACK_ADDR)] =
                stackmap->addr.phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
        DeleteTempVirtualMap(stackmap);

        /* Now it's time to copy the executable. */
        for (Size i = 0; i < code_size; i += PAGE_SIZE) {
                VirtualMap *codemap = CreateTempVirtualMap();

                ((PageTableEntry *)
                         ptmap->addr.virt)[((uintptr_t)FLAT_BINARY_DEFAULT_ENTRY + i) / PAGE_SIZE] =
                        codemap->addr.phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;
                kzeromem((void *)codemap->addr.virt, PAGE_SIZE);

                Size copy_size = (code_size - i < PAGE_SIZE) ? (code_size - i) : PAGE_SIZE;
                memcpy((char *)codemap->addr.virt, (char *)code + i, copy_size);

                DeleteTempVirtualMap(codemap);
        }

        /* And ensure it is present on the new address space. */
        ((PageDirectory *)pdmap->addr.virt)[0] =
                ptmap->addr.phys | PAGE_PRESENT | PAGE_RW | PAGE_USER;

        /* Also ensure the proper page table is set at the last PD index, as per the so commonly
         * typed in the source "recursive paging trick". */
        ((PageDirectory *)pdmap->addr.virt)[MAX_PD_ENTRIES - 1] =
                pdmap->addr.phys | PAGE_PRESENT | PAGE_RW;

        p->cr3         = pdmap->addr.phys;
        p->main_thread = main_thread;
        p->pid         = process_id_counter++;
        p->next        = NullPointer;
        p->ports_used  = 0;
        ZeroMemory(p->ioports);
        kzeromem(&p->handles, sizeof(HandleTable));

        main_thread->owner = p;

        DeleteTempVirtualMap(pdmap);
        DeleteTempVirtualMap(ptmap);

        AddThreadToScheduler(main_thread);
        if (process_list) {
                Process *iter = process_list;
                while (iter->next) iter = iter->next;
                p->prev    = iter;
                iter->next = p;
        } else {
                process_list = p;
                p->prev      = NullPointer;
        }
        return p;
}

Status DeleteProcess(Process *p) {
        DeleteProcessFromIRQRelay(p);

        if (p->prev)
                p->prev->next = p->next;
        else
                process_list = p->next;

        if (p->next) p->next->prev = p->prev;

        p->next = NullPointer;
        p->prev = NullPointer;

        for (int i = 0; i < MAX_OBJ_PER_PROCESS; i++) {
                Object *ob = DeleteFromHandleTable(&p->handles, i);
                if (ob) DeleteObject(ob);
        }

        static const char *no_vmem_msg = "Not enough virtual memory left for a temporary mapping";
        int                pd_slot     = FindFreeTmpMapSlot();
        if (pd_slot < 0) FatalError(no_vmem_msg);
        MarkTmpMapUsed((Size)pd_slot);
        int pt_slot = FindFreeTmpMapSlot();
        if (pt_slot < 0) FatalError(no_vmem_msg);

        uintptr_t pd_virt = TmpMapAddrFromIndex((Size)pd_slot);
        uintptr_t pt_virt = TmpMapAddrFromIndex((Size)pt_slot);

        MapSinglePage(p->cr3, pd_virt, PAGE_PRESENT | PAGE_RW);
        PageDirectory *pd = (PageDirectory *)pd_virt;

        p->main_thread->state = THREAD_TERMINATED;
        for (unsigned int i = 0; i < KERNEL_PD_INDEX; i++) {
                if (pd[i] & PAGE_PRESENT) {
                        Status mapstatus = MapSinglePage(pd[i] & PAGE_FRAME_MASK, pt_virt,
                                                         PAGE_PRESENT | PAGE_RW);
                        if (mapstatus != STATUS_OK) {
                                LogMessage(LOG_ERROR,
                                           "Unable to map page 0x%x to current virtual address "
                                           "space, status: \"%s\"",
                                           pt_virt, StatusCodeToString(mapstatus));
                        }
                        PageTableEntry *pt = (PageTableEntry *)pt_virt;

                        for (unsigned int j = 0; j < MAX_PT_ENTRIES; j++)
                                if (pt[j] & PAGE_PRESENT) FreeFrame(pt[j] & PAGE_FRAME_MASK);

                        UnmapSinglePage(pt_virt);
                        FreeFrame(pd[i] & PAGE_FRAME_MASK);
                }
        }

        UnmapSinglePage(pd_virt);
        MarkTmpMapFree((Size)pd_slot);
        MarkTmpMapFree((Size)pt_slot);

        for (int i = 0; i < KERNEL_STACK_SIZE_PAGES; i++) FreeFrame(p->kstacks[i].phys);

        FreeFrame(p->cr3);
        kfree(p);
        return STATUS_OK;
}

void SetProcessCapabilities(Process *process, u32 flags) { process->flags |= flags; }

[[gnu::pure]]
Process *FindProcessByID(ProcessID id) {
        if (unlikely(!process_list)) return NullPointer;

        Process *iter = process_list;
        while (iter) {
                if (iter->pid == id) return iter;
                iter = iter->next;
        }
        return NullPointer;
}
