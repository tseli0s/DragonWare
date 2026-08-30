/**********************************************************************
 * FILE: sysquery.h
 * PURPOSE: System configuration constants and values query
 * PROJECT: DragonWare Kernel
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief A system value used by the kernel. Usually referring to constants like page size, amount
 * of CPUs that can be used, and others.
 * @since v0.0.2
 */
typedef enum _SystemQuery : u32 {
        SQ_NONE                  = 0,
        SQ_N_OBJECTS_PER_PROCESS = 1, /** << Amount of objects per process  */
        SQ_PAGE_SIZE = 2, /** << Page size used by the kernel's virtual memory manager */
        SQ_N_CPUS    = 3, /** << Reserved  */
        SQ_TIMER_HZ =
                4, /** << Clock rate of the timer used by the kernel to perform quantum tracking */
        SQ_DEBUG_BUILD = 5, /** Whether this is a debug build or not  */
} SystemQuery;

/**
 * @brief Queries a system value @p key and returns the kernel-configured value for it. If extra
 * data needs to be copied, it is copied to the memory address pointed to by @p store (Currently
 * unused as of v0.0.2)
 * @param key The system value to query. See @ref SystemQuery
 * @param[out] store Pointer to memory where extra data will be copied, if needed, otherwise
 * ignored.
 * @returns The value of @p key as a system value upon the time of the call.
 */
int _DWSystemQuery(SystemQuery key, void *store);
