/**********************************************************************
 * FILE: section.h
 * PURPOSE: Section object implementation
 * PROJECT: DragonWare Kernel
 * DATE: 05-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "section.h"

#include <assert.h>
#include <kmalloc.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>
#include <panic.h>

#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "macros.h"
#include "mem/frame.h"
#include "sched/schedule.h"

/* Scans the address space to find a region of at least n_pages that are NOT mapped. Used to find
 * where to map a section. */
static uintptr_t FindFreePageRange(Size n_pages) {
        uintptr_t start = 0;
        Size      count = 0;

        /* First 16KBs of memory will be left alone, to catch bad pointers and other stuff. */
        for (uintptr_t addr = 4 * PAGE_SIZE; addr < KERNEL_VM_BASE; addr += PAGE_SIZE) {
                if (!IsVirtualPageMapped(addr)) {
                        if (count == 0) start = addr;
                        count++;

                        if (count == n_pages) return start;
                } else
                        count = 0;
        }
        return 0;
}

Section *AllocateSection(Size needed_pages, SectionPermissions permissions) {
        if (!needed_pages) return NullPointer;
        if (needed_pages >= MAX_SECTION_FRAMES) return NullPointer;

        Section *scn = kmalloc(sizeof(Section));
        if (!scn) return NullPointer;

        scn->address_space = 0; /* Set by the system calls later. */
        scn->permissions   = permissions;
        scn->n_pages       = needed_pages;
        scn->refcnt        = 0; /* Incremented only during MapSection(). */
        scn->vmbase        = 0;

        ZeroMemory(scn->physframes); /* VERY IMPORTANT, see MapSection below. */
        return scn;
}

uintptr_t MapSection(Section *section, Bool copy_on_write) {
        UnusedParameter(copy_on_write);
        if (!section) return 0;

        uintptr_t start = FindFreePageRange(section->n_pages);
        if (!start) return 0; /* Section cannot be mapped */

        u32 flags = PAGE_PRESENT | PAGE_USER;
        if (section->permissions & SECTION_WRITEABLE) flags |= PAGE_RW;
        if (!(section->permissions & SECTION_CACHEABLE)) {
                flags |= PAGE_CACHE_DISABLED;
                flags |= PAGE_WRITETHROUGH;
        }

        Size frames_allocated = 0;
        Size pages_mapped     = 0;

        for (Size i = 0; i < section->n_pages; i++) {
                Bool      previously_allocated = section->physframes[i] > 0;
                uintptr_t pageaddr             = start + (i * PAGE_SIZE);

                if (!previously_allocated) {
                        section->physframes[i] = AllocateFrame();
                        if (!section->physframes[i])
                                goto fail;
                        else
                                frames_allocated++;
                }

                if (MapSinglePage(section->physframes[i], pageaddr, flags) != STATUS_OK)
                        goto fail;
                else {
                        if (!previously_allocated) kzeromem((void *)pageaddr, PAGE_SIZE);
                        pages_mapped++;
                }
        }
        goto success;
fail:
        DEREF(section, {
                for (Size i = 0; i < frames_allocated; i++) FreeFrame(section->physframes[i]);
                for (Size i = 0; i < pages_mapped; i++) UnmapSinglePage(start + (i * PAGE_SIZE));
                ZeroMemory(section->physframes);
        });
        return 0;

success:
        REFER(section);
        section->vmbase = start;
        return start;
}

void UnmapSection(Section *section, void *base) {
        kassert(((uintptr_t)base) > 0);
        kassert(isaligned((uintptr_t)base, PAGE_SIZE));

        DEREF(section, {
                if (!section->n_pages) return; /* >> */
                for (Size i = 0; i < section->n_pages; i++) {
                        uintptr_t addr = ((uintptr_t)base) + (i * PAGE_SIZE);
                        kzeromem((void *)addr, PAGE_SIZE);
                        FreeFrame(section->physframes[i]);
                        section->physframes[i] = 0;
                        UnmapSinglePage(addr);
                }
        });
}

void DeleteSection(Section *section) {
        kassert(section->vmbase > 0);
        if (section->address_space != GetCurrentExecutionThread()->id) return;

        DEREF(section, {
                UnmapSection(section, (void *)section->vmbase);
                kfree(section);
        });
}
