/**********************************************************************
 * FILE: ipc.h
 * PURPOSE: Synchronous Inter Process Communication primitives
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "task/message.h"

/* message_size is not used yet */
Status _DWIPCSend(int handle, Message *m, u32 message_size);

Status _DWIPCReceive(int handle, Message *msave);
