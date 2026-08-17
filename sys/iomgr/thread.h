/**********************************************************************
 * FILE: thread.h
 * PURPOSE: Thread object related exports
 * PROJECT: DragonWare Kernel
 * DATE: 07-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "iomgr/object.h"

/**
 * @brief Data describing thread initial state for use with thread objects.
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _UserThreadData {
        void  (*entry)(void *); /** << Entry point of a thread  */
        void *stack;            /** << Stack memory for the thread*/
        void *extra_data;       /** << Extra data that will be given to the thread upon execution.*/
} UserThreadData;

/**
 * @brief Create a new thread object, setting up the @p stack for it and, when scheduled, begin
 * execution in @p entryaddr for it.
 *
 * @param[in] entryaddr Thread entry point. Must be a pointer to a function where execution will
 * begin.
 * @param[in] stack Address of the stack to load for the thread. Cannot be a @ref NullPointer
 * @param[in] extra_data Pointer to be passed to the new thread's entry point. Not checked by the
 * kernel.
 * @note The resulting constructed thread is NOT put in the scheduler list.
 * @returns A thread object as internal data for a @ref Object, or @ref NullPointer on failure.
 */
[[gnu::nonnull]]
void *CreateThread(void (*entryaddr)(void *), void *stack, void *extra_data);

/**
 * @brief Appends @p thread to the scheduler list, preparing it for execution.
 * @param[in] thread The thread to use. Must not be a @ref NullPointer
 */
[[gnu::nonnull]]
void RunThread(Object *thread);
