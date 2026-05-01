/**********************************************************************
 * FILE: vmm.c
 * PURPOSE: Virtual memory manager and implementation for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "vmm.h"

#include <ktypes.h>
#include <mmutils.h>
#include <panic.h>

#include "ddk/ia32/cpu.h"
#include "ddk/ia32/kcpuid.h"
#include "log.h"
#include "macros.h"
#include "mem/frame.h"
#include "paging.h"

#define KERNEL_VM_FLAGS   (PAGE_PRESENT | PAGE_RW)
#define KERNEL_RO_FLAGS   (PAGE_PRESENT)
#define KERNEL_MMIO_FLAGS (PAGE_PRESENT | PAGE_RW | PAGE_CACHE_DISABLED | PAGE_WRITETHROUGH)
#define KERNEL_PDE_FLAGS  (PAGE_PRESENT | PAGE_RW)

#define ONE_MEGABYTE      (0x00100000UL) /* Cooler assertion message :P */

extern char _begin;         /* Begin address of the kernel */
extern char _end;           /* End address of the kernel */
extern void FlushTLB(void); /* >> */

static inline void invlpg(const uintptr_t addr) {
        __asm__ volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

/* Should only be called if the processor supports PGE. Otherwise it may throw a #GPF */
static inline void EnablePGEControlBit(void) {
        __asm__ volatile(
                "movl %%cr4, %%eax\n"
                "orl $0x80, %%eax\n"
                "movl %%eax, %%cr4\n" ::
                        : "eax", "memory");
}

[[gnu::hot]]
static inline PageTableEntry *GetPageTableAt(u32 index) {
        if (unlikely(index >= MAX_PT_ENTRIES)) return NullPointer;
        return (PageTableEntry *)(((uintptr_t)PAGE_TABLE_BASE) + (index * PAGE_SIZE));
}

Status InitVirtualMemoryManager(void) {
        /* Page size alignment is extremely useful, otherwise we might miss any
         * data after the last page-aligned address.*/
        const Size kernel_size = pagealign((&_end - &_begin));

        PageDirectory  *pd = (PageDirectory *)AllocateLowMemory();
        PageTableEntry *pt = (PageTableEntry *)AllocateLowMemory();

        if (!pd || !pt) return STATUS_OUT_OF_MEMORY;

        /* We've identity mapped the first 1 megabyte, so it's safe to do this
         * directly. In the future, of course, this will need to be updated. */
        kzeromem((void *)pd, PAGE_SIZE);
        kzeromem((void *)pt, PAGE_SIZE);

        u32 flagbits    = KERNEL_VM_FLAGS;
        u32 flagbits_ro = KERNEL_RO_FLAGS;
        if (x86FeatureSupported(X86_PGE)) {
                LogMessage(LOG_INFO,
                           "This machine supports global TLB entries, using them for the kernel "
                           "address space.");
                EnablePGEControlBit();
                flagbits |= PAGE_GLOBAL;
                flagbits_ro |= PAGE_GLOBAL;
        }
        for (Size i = 0; i < kernel_size; i += PAGE_SIZE) {
                Size      ptindex = PT_INDEX(i);
                uintptr_t phys    = (virt_to_phys((uintptr_t)&_begin + i));
                pt[ptindex]       = phys | flagbits;
        }

        /* Allocate ALL the page tables that the kernel is going to use. This is very useful for
         * virtual mapping synchronization across processes' mapped kernel "version", because the
         * kernel remains the exact same across all processes (even if it doesn't use the actual
         * page tables for the most part).
         *
         * We are obviously preallocating a megabyte of memory, most of which we are not going to
         * use. But the speed benefits are worth it.
         * We already mapped the kernel in the loop above, so here we only need to allocate post the
         * kernel-occupied page tables. */
        for (Size i = KERNEL_PD_INDEX; i < MAX_PD_ENTRIES - 1; i++) {
                if (unlikely(!(pd[i] & PAGE_PRESENT))) {
                        PageTableEntry *ptnext = (PageTableEntry *)AllocateFrame();
                        kzeromem((void *)ptnext, FRAME_SIZE);
                        pd[i] = (PageDirectory)ptnext | KERNEL_PDE_FLAGS;
                }
        }
        /*
         * Mark .text and .rodata as read only, to catch any bad writes.
         * The symbols are declared in the linker script, we only need their addresses.
         */
        extern uintptr_t __text_section_start;
        extern uintptr_t __text_section_end;
        extern uintptr_t __rodata_section_start;
        extern uintptr_t __rodata_section_end;
        for (Size i = (Size)&__text_section_start; i < (Size)&__text_section_end; i += PAGE_SIZE) {
                uintptr_t ptindex = PT_INDEX(i);
                uintptr_t phys    = virt_to_phys(i);
                pt[ptindex]       = phys | flagbits_ro;
        }

        for (Size i = (Size)&__rodata_section_start; i < (Size)&__rodata_section_end;
             i += PAGE_SIZE) {
                uintptr_t ptindex = PT_INDEX(i);
                uintptr_t phys    = virt_to_phys(i);
                pt[ptindex]       = phys | flagbits_ro;
        }

        pd[KERNEL_PD_INDEX] = (uintptr_t)pt | KERNEL_PDE_FLAGS;

        /*
         * Recursive paging trick: https://wiki.osdev.org/User:Neon/Recursive_Paging
         * In simple terms, put the page directory at the last page of the address space,
         * so accessing any entry in it is simply incrementing a pointer and adding it to the
         * virtual address */
        pd[MAX_PD_ENTRIES - 1] = (uintptr_t)pd | KERNEL_PDE_FLAGS;

        SetPageDirectory((uintptr_t)pd);

        return STATUS_OK;
}

Status MapSinglePage(uintptr_t phys, uintptr_t virt, u32 flags) {
        if (unlikely(virt == 0x0 || !isaligned(phys, PAGE_SIZE) || !isaligned(virt, PAGE_SIZE)))
                return STATUS_BAD_ARGUMENT;

        const u32 pdindex = PD_INDEX(virt);
        const u32 ptindex = PT_INDEX(virt);

        PageDirectory *pd = CURRENT_PAGE_DIRECTORY;

        u32 pdflags = KERNEL_PDE_FLAGS;
        if (flags & PAGE_USER) pdflags |= PAGE_USER;

        /* This will only run for addresses below KERNEL_VM_BASE because we have preallocated all
         * the page tables for the kernel */
        if (unlikely(!(pd[pdindex] & PAGE_PRESENT))) {
                uintptr_t new_pt_phys = AllocateLowMemory();
                if (!new_pt_phys) return STATUS_OUT_OF_MEMORY;

                pd[pdindex]        = (uintptr_t)new_pt_phys | pdflags;
                uintptr_t pt_vaddr = (uintptr_t)GetPageTableAt(pdindex);
                invlpg(pt_vaddr);
                kzeromem((void *)pt_vaddr, PAGE_SIZE);
        }

        PageTableEntry *pt = GetPageTableAt(pdindex);

        if (unlikely(pt[ptindex] & PAGE_PRESENT)) return STATUS_RETRY;

        pt[ptindex] = phys | flags;
        invlpg(virt);
        return STATUS_OK;
}

void UnmapSinglePage(uintptr_t virt) {
        const Size pdindex = PD_INDEX(virt);
        const Size ptindex = PT_INDEX(virt);

        PageDirectory *pd = CURRENT_PAGE_DIRECTORY;
        if (unlikely(!(pd[pdindex] & PAGE_PRESENT))) return;

        PageTableEntry *pt = GetPageTableAt(pdindex);

        if (unlikely(!(pt[ptindex] & PAGE_PRESENT))) return;

        pt[ptindex] = 0;
        invlpg(virt);
}

Bool IsVirtualPageMapped(uintptr_t addr) {
        Size pdindex = PD_INDEX(addr);
        Size ptindex = PT_INDEX(addr);

        if (!(CURRENT_PAGE_DIRECTORY[pdindex] & PAGE_PRESENT)) return false;

        PageTableEntry *pt = GetPageTableAt(pdindex);
        return (pt[ptindex] & PAGE_PRESENT);
}

Size MapMemoryRange(uintptr_t phys, uintptr_t virt, u32 flags, Size n_pages) {
        Size n = 0;
        for (Size i = 0; i < n_pages; i++) {
                if (likely(MapSinglePage(phys + i * PAGE_SIZE, virt + i * PAGE_SIZE, flags) ==
                           STATUS_OK))
                        n++;
        }
        return n;
}

Status UnmapMemoryRange(uintptr_t virt, Size n_pages) {
        if (!isaligned(virt, PAGE_SIZE)) return STATUS_BAD_ARGUMENT;

        Bool any_unmapped_pages = false;
        for (Size i = 0; i < n_pages; i++) {
                uintptr_t target = virt + (i * PAGE_SIZE);
                if (unlikely(!IsVirtualPageMapped(target))) {
                        any_unmapped_pages = true;
                        continue;
                }
                UnmapSinglePage(target);
        }
        return (any_unmapped_pages) ? STATUS_OUT_OF_BOUNDS : STATUS_OK;
}
