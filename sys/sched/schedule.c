/**********************************************************************
 * FILE: schedule.c
 * PURPOSE: Round robin scheduling implementation
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "sched/schedule.h"

#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <panic.h>

#include "ddk/ia32/ctxswitch.h"
#include "task/task.h"

/**
 * @brief Runs a given block of code for each thread registered with the scheduler.
 * @param[in] __LAMBDA__ The piece
 * @note The currently iterated thread on each loop iteration is called @code thr @endcode which is
 * how to access the thread being iterated
 */
#define ForEachScheduledThread(__LAMBDA__) \
        do {                               \
                Thread *thr = thread_list; \
                while (thr) {              \
                        __LAMBDA__         \
                        thr = thr->next;   \
                }                          \
        } while (0)

typedef struct _ReadyQueue {
        Thread *head;
        Thread *tail;
} ReadyQueue;

static Thread *thread_list    = NullPointer;
static Thread *current_thread = NullPointer;

/* WARNING: Global state variable. This will only be accessed when interrupts are disabled. */
[[gnu::visibility("default")]]
volatile int NeedsResched = 0;

/**
 * @brief Compares a @ref ThreadPriorityLevel level and returns, in timer ticks, the amount of
 * process time it is going to get.
 * @param level The thread level to compare against. See @ref ThreadPriorityLevel for allowed
 * values.
 * @return The amount of CPU time that the process will get in timer ticks, depending on its
 * priority.
 */
static inline u32 CPUTimeFromThreadPriority(ThreadPriorityLevel level) {
        switch (level) {
                case NO_PRIORITY:
                        return 0;
                case PRIORITY_LOW:
                        return 1;
                case PRIORITY_MEDIUM:
                        return 12;
                case PRIORITY_HIGH:
                        return 20;
                case PRIORITY_MAX:
                        return 33;
                default:
                        return 60;
        }
}

[[gnu::hot]]
static Thread *SelectNextThread(void) {
        if (!thread_list) return NullPointer;
        if (!current_thread) current_thread = thread_list;

        Thread *start = (current_thread->next) ? current_thread->next : thread_list;
        Thread *t     = start;
        Thread *idle  = NullPointer;

        do {
                if (t->state != THREAD_BLOCKED && t->state != THREAD_TERMINATED &&
                    t->state != THREAD_RUNNING) {
                        if (t->priority == PRIORITY_IDLE) {
                                if (!idle) idle = t;
                        } else {
                                t->quantum = CPUTimeFromThreadPriority(t->priority);
                                return t;
                        }
                }

                t = (t->next) ? t->next : thread_list;

        } while (t != start);

        if (idle) {
                idle->quantum = CPUTimeFromThreadPriority(idle->priority);
                return idle;
        }

        return NullPointer;
}

Status InitSchedulerState(void) {
        if (!thread_list) return STATUS_RETRY;

        ForEachScheduledThread({
                thr->quantum = CPUTimeFromThreadPriority(thr->priority);
                thr->state   = THREAD_READY;
        });
        current_thread->state = THREAD_RUNNING;
        current_thread->quantum++;
        return STATUS_OK;
}

[[gnu::hot]]
void ScheduleNext(void) {
        if (unlikely(!thread_list)) return;
        Thread *prev                  = current_thread;
        Thread *next                  = SelectNextThread();
        Bool    should_switch_process = true;

        NeedsResched = 0;

        /* There are no other threads probably. Keep running the same one. */
        if (unlikely(!next)) {
                current_thread = prev;
                return;
        }
        if (likely(prev != next)) {
                if (prev->state == THREAD_RUNNING) prev->state = THREAD_READY;
                next->state    = THREAD_RUNNING;
                next->quantum  = CPUTimeFromThreadPriority(next->priority);
                current_thread = next;
                if (prev && prev->owner && prev->owner == current_thread->owner)
                        should_switch_process = false;
                if (current_thread->owner && should_switch_process)
                        SwapProcess(current_thread->owner);

                SwapThreadStack(&prev->esp, next->esp);
        }
}

void AddThreadToScheduler(Thread *thread) {
        thread->next = NullPointer;
        if (!thread_list) {
                thread_list    = thread;
                current_thread = thread;
                return;
        }

        Thread *iter = thread_list;
        while (iter->next) iter = iter->next;
        iter->next = thread;
}

Status RemoveThreadFromScheduler(Thread *t) {
        if (!thread_list) return STATUS_NOT_FOUND;

        if (t == thread_list) {
                thread_list = thread_list->next;
                return STATUS_OK;
        }

        Thread *iter = thread_list;
        while (iter->next) {
                if (iter->next == t) {
                        iter->next = t->next;
                        return STATUS_OK;
                }
                iter = iter->next;
        }

        return STATUS_NOT_FOUND;
}

void DropQuantumOfCurrentThread(void) { current_thread->quantum = 0; }

[[noreturn]]
void _ThreadExitProc() {
        current_thread->state = THREAD_TERMINATED;
        current_thread->esp   = current_thread->stack_top; /* Reset the stack too. Maybe zero it? */
        YieldCurrentThread();
        while (true) __asm__ volatile("pause");
}

Thread *GetCurrentExecutionThread(void) { return current_thread; }

Thread *GetThreadByID(ThreadID id) {
        if (!thread_list) return NullPointer;
        Thread *walk = thread_list;
        while (walk) {
                if (walk->id == id) return walk;
                walk = walk->next;
        }

        return NullPointer;
}

Thread *GetSchedulerThreadList(void) { return thread_list; }

void SwapSchedulerThreadState(Thread *new) {
        Thread *current = current_thread;
        if (new->owner != current->owner) SwapProcess(new->owner);
        new->quantum = CPUTimeFromThreadPriority(new->priority);
        new->state   = THREAD_RUNNING;

        current_thread = new;
        SwapThreadStack(&current->esp, new->esp);
}
