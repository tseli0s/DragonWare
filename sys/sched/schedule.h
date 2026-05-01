/**********************************************************************
 * FILE: schedule.h
 * PURPOSE: Round robin scheduling implementation exports
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "task/task.h"

/**
 * @brief Initializes the scheduler state and prepares the kernel for task scheduling.
 * @sa ScheduleNext
 * @returns @ref STATUS_OK if the initialization was successful, and @ref STATUS_RETRY if the
 * scheduler is not ready to be initialized yet (eg. No threads that can be ran)
 */
Status InitSchedulerState(void);

/**
 * @brief Schedules a new thread to run and switches the stacks to the ones expected by it.
 * @warning This function should only be called from specific paths. It is also very expensive to
 * run (Walks an entire linked list of threads to find one that will get CPU time). It will also
 * perform a context switch directly, meaning the next instructions may be in a completely different
 * thread until the calling thread is rescheduled.
 */
void ScheduleNext(void);

/**
 * @brief Adds a new thread to the list of threads that will be selected for CPU time by the
 * scheduler
 * @param[in] thread The thread to add, must not be @ref NullPointer
 */
[[gnu::nonnull(1)]]
void AddThreadToScheduler(Thread* thread);

/**
 * @brief Remove a thread from the scheduler, effectively making it unable to run ever again
 * @param[in] t The thread to remove, must not be a @ref NullPointer
 * @returns STATUS_OK on success, or another @ref Status code on failure
 */
[[gnu::nonnull(1)]]
Status RemoveThreadFromScheduler(Thread* t);

/** @brief Sets the CPU time left for the current thread to 0, effectively forcing the scheduler to
 * pick a new process next. */
void DropQuantumOfCurrentThread(void);

/**
 * @brief Thread exit procedure. Not to be used directly.
 */
[[noreturn]]
void _ThreadExitProc(void);

/**
 * @brief Fetch the thread currently being executed on the CPU
 * @return The thread that currently receives CPU time
 */
Thread* GetCurrentExecutionThread(void);

/**
 * @brief Find a thread with the given ID and return it
 * @warning This is a very expensive function, call it carefully.
 * @note This only works with threads already added for scheduling. Newly created threads must be
 * added first.
 * @param id The ID to look for.
 * @returns The thread with the given ID on success, or NullPointer if the thread couldn't be found.
 */
Thread* GetThreadByID(ThreadID id);

/**
 * @brief Fetch the list of threads within the scheduler (Regardless of their state). Used by the
 * idle thread to clean up any terminated threads.
 * @returns The list of threads that can be scheduled.
 */
Thread* GetSchedulerThreadList(void);

/**
 * @brief Switches from the current thread to thread @p new (And process, if the current thread is
 * part of a different process), and updates the scheduler to know the new thread that is running.
 * @param[in] new The new thread to switch to, must not be NullPointer.
 * @note This also changes the @p new thread's quantum depending on its priority.
 */
[[gnu::nonnull]]
void SwapSchedulerThreadState(Thread* new);
