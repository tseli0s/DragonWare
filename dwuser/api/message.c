/**********************************************************************
 * FILE: message.c
 * PURPOSE: IPC messaging wrappers for DragonWare applications
 * PROJECT: DragonWare User Library
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kerneltypes.h>
#include <message.h>

Status SendMessage(Handle handle, Message *m, Size message_size) {
        if (handle < 0 || handle >= 64)
                return STATUS_OUT_OF_BOUNDS; /* 64 is the maximum supported by the kernel so far in
                                                some configurations. */
        if (message_size < sizeof(m->header)) return STATUS_BAD_ARGUMENT;

        return _DWIPCSend(handle, m, message_size);
}

Status ReceiveMessage(Handle handle, Message *msave) {
        if (handle < 0 || handle >= 64) return STATUS_OUT_OF_BOUNDS;
        return _DWIPCReceive(handle, msave);
}
