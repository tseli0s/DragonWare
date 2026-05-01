/**********************************************************************
 * FILE: task.c
 * PURPOSE: Stack frame based task/thread implementation
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "task/task.h"

#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "ddk/ia32/gdt.h"
#include "sched/schedule.h"

#define DEFAULT_THREAD_STACK (1024)
#define DEFAULT_EFLAGS \
        (0x202) /* Bits IF and reserved set. Perhaps this should be preserved, anyways. */

/* Controls whether the scheduler will run when returning from an interrupt */
extern volatile int NeedsResched;

/* Defined in task.asm */
[[noreturn]]
extern void _ThreadStalled(void);

/* Also defined in task.asm */
extern void _ThreadStartup(u32 trapframe);

/* Incremented every new thread */
static ThreadID next_id = 0;

/* This is the first function called when a thread is entered for the first time. */
[[noreturn]]
static void _ThreadBootstrapTrampoline(void) {
        /* If we have reached this function, then the scheduler already switched the current thread
         * it wants us to run, so we are not by accident using an old thread here. */
        Thread *curr = GetCurrentExecutionThread();
        _ThreadStartup(curr->trapframe);

        /* We should NEVER reach this point, but if we do, idk bad things happen and a scary black
         * monster will kill us  */
        while (1) {
        }
}

Thread *AllocateThread(ThreadEntryPoint entry, void *_unused) {
        UnusedParameter(_unused);

        Thread *t = kmalloc(sizeof(Thread));
        if (!t) return NullPointer;

        /* Pointer arithmetic rules in C advance the address by the size of the type of the pointer,
         * not by a single byte, which is why the division below is a thing */
        u32 *stack_bottom = kmalloc(DEFAULT_THREAD_STACK);
        u32 *esp          = stack_bottom + (DEFAULT_THREAD_STACK / sizeof(u32));
        kzeromem(stack_bottom, DEFAULT_THREAD_STACK);

        /* Automatically delete the thread if it returns */
        *(--esp) = (u32)_ThreadExitProc;

        /* Volatile state. This is used by SwapThread(). */
        *(--esp) = (u32)entry;
        *(--esp) = 0; /* EBP */
        *(--esp) = 0; /* EBX */
        *(--esp) = 0; /* ESI */
        *(--esp) = 0; /* EDI */

        /* Now the thread frame is ready, set it up */
        t->esp = t->stack_top = (u32)esp;
        t->state              = THREAD_READY;
        t->owner              = _unused;
        t->next               = NullPointer;
        t->pl                 = THREAD_KERNEL;
        t->priority           = PRIORITY_LOW; /* Kernel threads don't need a lot of time */
        t->quantum            = 6;
        t->id                 = next_id++;
        return t;
}

Thread *AllocateUserThread(ThreadEntryPoint entry, uintptr_t useresp, uintptr_t kernel_stack) {
        Thread *t = kmalloc(sizeof(Thread));
        if (!t) return NullPointer;

        /* TODO: Allocate a user-mapped page here instead. */
        u32 *esp = (u32 *)kernel_stack;

        /* Thread stack frame. iret pops ss, useresp, eflags, cs, and eip when iret is called */
        *(--esp) = SEL_DATA_USER;  /* userss, simply the same as the rest */
        *(--esp) = useresp;        /* Set the process' stack while we're at it */
        *(--esp) = DEFAULT_EFLAGS; /* default eflags only have IF set */
        *(--esp) = SEL_CODE_USER;  /* cs, must be user selector of course */
        *(--esp) = (u32)entry;     /* eip set to the entry point as soon as we iret */

        /* Now save the trap frame used to return into user space. */
        t->trapframe = (u32)esp;

        /* Volatile state. This is used by SwapThread(). */
        *(--esp) = (u32)_ThreadBootstrapTrampoline;
        *(--esp) = 0; /* EBP */
        *(--esp) = 0; /* EBX */
        *(--esp) = 0; /* ESI */
        *(--esp) = 0; /* EDI */

        /* Now the thread frame is ready, set it up */
        t->esp = t->stack_top = (u32)esp;
        t->state              = THREAD_READY;
        t->owner              = NullPointer; /* Set by CreateProcess */
        t->next               = NullPointer;
        t->pl                 = THREAD_USER;
        t->priority           = PRIORITY_MEDIUM;
        t->quantum            = 12; /* The scheduler will update it later */
        t->id                 = next_id++;
        return t;
}

void YieldCurrentThread(void) {
        NeedsResched = 1;
#ifdef __i386__
        __asm__ volatile("int $0x20");
#endif /* __i386__ */
}

void DeleteThread(Thread *t) {
        t->state = THREAD_TERMINATED;
        if (GetCurrentExecutionThread() == t) return; /* Let somebody else clean us up */
        if (unlikely(RemoveThreadFromScheduler(t) == STATUS_NOT_FOUND)) {
                LogMessage(LOG_ERROR,
                           "Attempted to remove thread %p from scheduler, but it wasn't found in "
                           "the scheduling list!",
                           t);
                return;
        }
}

void BlockThread(Thread *t) {
        t->state     = THREAD_BLOCKED;
        NeedsResched = 1;
}

void WakeThread(Thread *t) {
        if (t->state != THREAD_RUNNING) {
                t->state     = THREAD_READY;
                NeedsResched = 1;
        }
}
