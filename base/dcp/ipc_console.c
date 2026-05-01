/**********************************************************************
 * FILE: ipc_console.c
 * PURPOSE: Console IPC helpers implementation
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "ipc_console.h"

#include <message.h>
#include <string.h>

#include "vgacons/protocol.h"

/* XXX This should be replaced by printf see FIXME below */
void PrintMessageToConsole(Handle console_handle, char *msg) {
        size_t msglen = strlen(msg);
        if (msglen > MESSAGE_BUFFER_SIZE)
                return; /* The console won't accept it anyways, no need to bother */

        Message send;

        send.header.protocol       = VGACONS_PROTOCOL_V0;
        send.header.type           = VGACONS_REQUEST_STRING_DRAW;
        send.header.payload_length = msglen + 1;
        send.header.reply_handle   = -1;
        strncpy((char *)send.payload.raw, msg, msglen);
        send.payload.raw[msglen] = '\0';

        SendMessage(console_handle, &send, sizeof(send.header) + send.header.payload_length);
}
