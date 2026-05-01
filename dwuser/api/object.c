/**********************************************************************
 * FILE: object.c
 * PURPOSE: Kernel object wrappers for user libraries
 * PROJECT: DragonWare User Library
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "object.h"

#include "kernelapi.h"

Handle CreateObject(const char *name, ObjectType type, u32 permissions) {
        return _DWCreateObject(name, type, permissions);
}

Status InvokeObject(Handle handle, ObjectOperation op, void *args) {
        return _DWInvokeObject(handle, op, args);
}

void DeleteObject(Handle handle) { _DWDeleteObject(handle); }
