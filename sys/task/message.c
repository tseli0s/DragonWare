/**********************************************************************
 * FILE: message.c
 * PURPOSE: Kernel to user process/thread messaging
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "message.h"

#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "iomgr/object.h"
#include "iomgr/port.h"
#include "process.h"
#include "task.h"

void EnqueueMessage(Port *port, Message *new) {
        Size index = port->tail;
        memcpy(&port->msgbuf[index], new, sizeof(MessageHeader) + new->header.payload_length);
        port->tail = (port->tail + 1) % MAX_MESSAGES_IN_PORT;
        port->count++;
}

[[gnu::nonnull, gnu::hot]]
Status SendMessage(Process *process, int handle, Message *msg) {
        /* Branch predictions significantly help here, since this is in kernel space we can verify
         * more or less what's going to happen. */
        if (unlikely(!inrange(handle, 0, MAX_OBJ_PER_PROCESS - 1))) return STATUS_OUT_OF_BOUNDS;
        Object *target = process->handles.objlist[handle];
        if (unlikely(target->type != OBJ_PORT || !target->data)) return STATUS_BAD_ARGUMENT;
        if (unlikely(msg->header.payload_length > MESSAGE_BUFFER_SIZE)) {
                LogMessage(LOG_WARNING,
                           "KERNEL BUG: Message payload length %d exceeds MESSAGE_BUFFER_SIZE %d",
                           msg->header.payload_length, MESSAGE_BUFFER_SIZE);
                return STATUS_OUT_OF_BOUNDS;
        }

        msg->header.sender       = KERNEL_SENDER;
        msg->header.reply_handle = -1; /* Never let a process message us back */
        msg->header.reserved     = 0;

        /* Standard queue insertion. TODO: This is almost identical to the code in
         * sys/syscall/ipc.c, create a helper function to do this for us. */
        Port *port = target->data;

        EnqueueMessage(port, msg);
        WakeThread(port->owner);

        return STATUS_OK;
}
