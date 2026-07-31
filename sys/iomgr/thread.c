/**********************************************************************
 * FILE: thread.c
 * PURPOSE: Thread object management implementation
 * PROJECT: DragonWare Kernel
 * DATE: 07-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "thread.h"

#include <ktypes.h>
#include <log.h>
#include <panic.h>

#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "iomgr/object.h"
#include "mem/frame.h"
#include "sched/schedule.h"
#include "task/process.h"
#include "task/task.h"

/* Returns a virtual address that is free to map the kernel stack for a process contiguously. Panics
 * if it can't find one. Copied verbatim from sys/task/process.c, so we probably need to apply some
 * DRY here. (<---- FIXME) */
static uintptr_t GetNextKernelStackAddress(void) {
        uintptr_t current_addr = KERNEL_STACK_BASE;

        /* The kernel stack is two pages wide for each process, which is why we need two contiguous
         * pages available. */
        while (current_addr + (2 * PAGE_SIZE) <= KERNEL_STACK_END) {
                if (!IsVirtualPageMapped(current_addr) &&
                    !IsVirtualPageMapped(current_addr + PAGE_SIZE)) {
                        return current_addr;
                }
                current_addr += (2 * PAGE_SIZE);
        }

        return 0;
}

void *CreateThread(void (*entryaddr)(void), void *stack) {
        /* Declaring it on top otherwise the bitch called clangd says "Variable
         * 't' is used uninitialized whenever 'if' condition is true" in the checks below */
        Thread *t = NullPointer;

        uintptr_t phys_stack_1 = AllocateFrame();
        uintptr_t phys_stack_2 = AllocateFrame();
        uintptr_t stackaddr    = GetNextKernelStackAddress() + (2 * PAGE_SIZE);

        if (!phys_stack_1 || !phys_stack_2) goto bad;
        if (MapSinglePage(phys_stack_2, stackaddr - PAGE_SIZE, PAGE_PRESENT | PAGE_RW) != STATUS_OK)
                goto bad;
        if (MapSinglePage(phys_stack_1, stackaddr - (2 * PAGE_SIZE), PAGE_PRESENT | PAGE_RW) !=
            STATUS_OK)
                goto bad;

        t = AllocateUserThread(entryaddr, (uintptr_t)stack, stackaddr);
        if (!t) goto bad;

        t->owner = GetCurrentExecutionThread()->owner;
        return t;

bad:
        if (phys_stack_1) FreeFrame(phys_stack_1);
        if (phys_stack_2) FreeFrame(phys_stack_2);
        UnmapSinglePage(stackaddr - PAGE_SIZE);
        UnmapSinglePage(stackaddr - (2 * PAGE_SIZE));
        if (t) DeleteThread(t);

        return NullPointer;
}

[[gnu::nonnull]]
void RunThread(Object *thread) {
        AddThreadToScheduler(thread->data);
}
