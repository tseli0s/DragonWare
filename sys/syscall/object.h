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

int    _DWCreateObject(const char *name, ObjectType type, u32 permissions);
Status _DWInvokeObject(int handle, unsigned long op, void *argptr);
void   _DWDeleteObject(int handle);
