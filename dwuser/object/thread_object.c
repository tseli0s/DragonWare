/**********************************************************************
 * FILE: thread_object.c
 * PURPOSE: Section object wrappers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <object.h>
#include <object/section_object.h>

#define STACK_PAGES_NEEDED (4)

Status SpawnThread(void (*__fn)(void*), void *data) {
        void *stack;
        Handle stackobj = RequestMemorySection(STACK_PAGES_NEEDED, SECTION_WRITEABLE | SECTION_SHAREABLE);
        Handle thread = -1;
        if (stackobj < 0) return STATUS_OUT_OF_MEMORY;

        if (MapMemorySection(stackobj, &stack) != STATUS_OK) goto bad_oom;

        UserThreadData thread_data = {
                .entry = __fn,
                .stack = stack,
                .extra_data = data
        };

        thread = CreateObject(NullPointer, OBJ_THREAD, 0);
        if (thread < 0) goto bad_oom;

        if (InvokeObject(thread, THREAD_CREATE, &thread_data) != STATUS_OK) goto bad_oom;
        if (InvokeObject(thread, THREAD_RUN, NullPointer) != STATUS_OK) goto bad_sched;
        return STATUS_OK;
        
bad_oom:
        if (thread >= 0) DeleteObject(thread);
        if (stackobj >= 0) DeleteObject(stackobj);
        return STATUS_OUT_OF_MEMORY;
bad_sched:
        if (thread >= 0) DeleteObject(thread);
        if (stackobj >= 0) DeleteObject(stackobj);
        return STATUS_RETRY;
}
