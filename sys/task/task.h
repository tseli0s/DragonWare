/**********************************************************************
 * FILE: task.h
 * PURPOSE: Stack frame based task/thread implementation
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "message.h"

typedef u32  ThreadID;
typedef void (*ThreadEntryPoint)(void);

typedef struct _MessageQueue {
        Message               m;
        struct _MessageQueue *prev;
        struct _MessageQueue *next;
} MessageQueue;

/* Forward declaration, declared/defined in process.h */
typedef struct _Process Process;

/** @brief Defines the state of a thread for the scheduler interface. */
typedef enum _ThreadState {
        THREAD_TERMINATED = 0, /* For binary logic mostly */
        THREAD_READY,          /* Thread is ready to start executing */
        THREAD_RUNNING,        /* Thread is running */
        THREAD_BLOCKED,        /* Thread does not deserve any CPU time */
} ThreadState;

/** @brief Defines the hardware-based privilege level of a thread */
typedef enum _ThreadPrivilegeLevel {
        THREAD_KERNEL = 0, /** << Kernel thread (CPL 0)  */
        THREAD_USER   = 3, /** << User task (CPL 3) */
} ThreadPrivilegeLevel;

/**
 * @brief An integer describing thread priority levels.
 * @details -1 means that the thread is not meant to be ran. 0 is the idle task. 1 gets 12ms of CPU
 * time. 2 gets 20ms of CPU time. 3 gets 33 ms of CPU time. Greater values get the maximum amount
 * of CPU time allowed, 60ms.
 */
typedef enum _ThreadPriorityLevel : signed short {
        NO_PRIORITY     = -1,
        PRIORITY_IDLE   = 0,
        PRIORITY_LOW    = 1,
        PRIORITY_MEDIUM = 2,
        PRIORITY_HIGH   = 3,
        PRIORITY_MAX    = 4,
} ThreadPriorityLevel;

typedef struct _Thread {
        u32 esp;       /* The current stack pointer */
        u32 trapframe; /* Trap frame. Contains the frame required to return into userspace (iret
                          frame in other words) */
        u32 stack_top; /* We keep a pointer to the original stack so we can just reset threads in
                          the future */
        u32 quantum;   /* CPU time of this process */
        ThreadState          state;
        ThreadPrivilegeLevel pl;
        ThreadPriorityLevel  priority;
        ThreadID             id;
        int                  waiting_on; /* To which port handle is this thread waiting on */
        Process        *owner; /* Owner of the thread. If NullPointer, this is a kernel thread. */
        struct _Thread *next;  /* Round-robin scheduler */
} Thread;

/**
 * @brief Allocates a new kernel thread, initializes it to a known state and returns it for use by
 * the scheduler.
 * @param[in] entry The routine to run in the new thread, see @ref ThreadEntryPoint
 * @param[in] _unused Reserved for future expansion. Must be a @ref NullPointer
 * @return The newly allocated thread, ready to be executed
 */
Thread *AllocateThread(ThreadEntryPoint entry, void *_unused);

/**
 * @brief Allocates an unprivileged (User) thread and prepares it for execution.
 * @param[in] entry The entry point to jump to (The program's entry point, usually)
 * @param[in] useresp Where to place the stack for the new address. Usually handled by @ref
 * CreateProcess
 * @param[in] kernel_stack The kernel stack loaded into the TSS, also used to store the thread
 * state.
 * @return A newly allocated unprivileged task to be scheduled. The thread is automatically added to
 * the scheduling list.
 */
Thread *AllocateUserThread(ThreadEntryPoint entry, uintptr_t useresp, uintptr_t kernel_stack);

/**
 * @brief Releases the CPU time of the current thread, allowing the scheduler to pick a new thread
 * at the next iteration.
 * @note The actual scheduling/context switching is not taking place here. This simply changes some
 * internal scheduler state.
 */
void YieldCurrentThread(void);

/**
 * @brief Deletes a thread and frees all the memory used up by it.
 * @param[in] t The thread to delete.
 */
[[gnu::nonnull]]
void DeleteThread(Thread *t);

/**
 * @brief Blocks the given thread from getting any CPU time (in other words, puts it to sleep)
 * @param[in] t The thread to block.
 */
[[gnu::nonnull]]
void BlockThread(Thread *t);

/**
 * @brief Allows a thread to get CPU time in the future (in other words, wakes it up)
 * @param[in] t The thread to wake
 */
[[gnu::nonnull]]
void WakeThread(Thread *t);
