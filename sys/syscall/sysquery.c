/**********************************************************************
 * FILE: sysquery.c
 * PURPOSE: System configuration constants and values query
 * PROJECT: DragonWare Kernel
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "sysquery.h"

#include <ktypes.h>
#include <macros.h>

#include "iomgr/object.h"
#include "time/timer.h"

/**
 * @brief Queries a system value @p key and returns the kernel-configured value for it. If extra
 * data needs to be copied, it is copied to the memory address pointed to by @p store (Currently
 * unused as of v0.0.2)
 * @param key The system value to query. See @ref SystemQuery
 * @param[out] store Pointer to memory where extra data will be copied, if needed, otherwise
 * ignored.
 * @returns The value of @p key as a system value upon the time of the call.
 */
int _DWSystemQuery(SystemQuery key, void *store) {
        UnusedParameter(store);
        switch (key) {
                case SQ_N_OBJECTS_PER_PROCESS:
                        return MAX_OBJ_PER_PROCESS;
                case SQ_PAGE_SIZE:
                        return PAGE_SIZE;
                case SQ_N_CPUS:
                        return -1;
                case SQ_TIMER_HZ:
                        return TARGET_HZ;
                case SQ_DEBUG_BUILD:
#ifdef DRAGONWARE_DEBUG_MODE
                        return 1;
#else
                        return 0;
#endif /* DRAGONWARE_DEBUG_MODE */
                case SQ_NONE:
                default:
                        return -1;
        }
        unreachable;
}
