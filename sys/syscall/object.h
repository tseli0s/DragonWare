/**********************************************************************
 * FILE: object.h
 * PURPOSE: Kernel object system call API implementation
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "iomgr/object.h"
#include "task/process.h"

int    _DWCreateObject(const char *name, ObjectType type, u32 permissions);
Status _DWInvokeObject(int handle, unsigned long op, void *argptr);
void   _DWDeleteObject(int handle);

/**
 * @brief Given a handle @p handle belonging to a process by ID @p process_id, duplicate the handle
 * and store it into @p save for the caller process.
 * @param process_id The ID of the process that @p handle belongs to. 0 is invalid.
 * @param handle The handle of the other process to translate into the current process.
 * @param[out] save Where to store the resulting handle on success. This is a user pointer.
 * @note The @p handle is translated and appended into the current process' handle table. The new
 * handle stored in @p save may be different than the original @p handle
 * @returns STATUS_OK if the handle was translated successfully. STATUS_NOT_FOUND if the handle does
 * not point to anything valid or the process with ID @p process_id is not found.
 * STATUS_BAD_ARGUMENT if one of the parameters is invalid. STATUS_OUT_OF_MEMORY if there are no
 * free slots to store the handle to. STATUS_BAD if the copy to @p save failed.
 */
Status _DWTranslateHandle(ProcessID process_id, int handle, int *save);
