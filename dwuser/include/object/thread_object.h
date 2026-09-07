/**********************************************************************
 * FILE: thread.h
 * PURPOSE: Thread object helpers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>

/**
 * @brief Spawns and runs a thread.
 * @details This function will spawn a new thread, thread meaning "unit of execution", and @b
 * immediately schedule it for execution. The entry point will be set to @p __fn and the extra
 * argument @p data will be passed to @p __fn.
 * @param[in] __fn Entry point of the thread (Where will execution start on the new thread). Should
 * not be @ref NullPointer.
 * @param[in] data Extra data to be given to @p __fn upon call. Ignored entirely by this function
 * otherwise.
 * @returns STATUS_OK if the thread was spawned and scheduled for execution successfully.
 * STATUS_OUT_OF_MEMORY if allocating the object or the thread's stack failed. STATUS_RETRY if the
 * thread couldn't be scheduled.
 */
Status SpawnThread(void (*__fn)(void *), void *data);
