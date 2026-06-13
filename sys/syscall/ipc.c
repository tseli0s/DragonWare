/**********************************************************************
 * FILE: ipc.c
 * PURPOSE: IPC userspace system calls implementation
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/* Okay, probably the IPC implementation is way too slow for now. Yeah I know. Honestly, it's my
 * first time doing IPC myself, so I know there's lots of room for improvement. In any case, this
 * code will be improved significantly in the future. */

#include "ipc.h"

#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>

#include "iomgr/object.h"
#include "iomgr/port.h"
#include "sched/schedule.h"
#include "task/message.h"
#include "task/process.h"
#include "task/task.h"
#include "usercopy.h"

#define MAX_QUEUED_MESSAGES (128)

/* XXX This is probably the backbone of all of DragonWare so it must be optimized down to the
 * tiniest performance gains.  */
[[gnu::hot]]
Status _DWIPCSend(int handle, Message *m, u32 message_size) {
        /* Truncate the message if a malicious process tried to copy more than the size of the
         * message */
        if (message_size > sizeof(Message)) message_size = sizeof(Message);
        /* Refuse to send the message if it doesn't include the header. */
        if (message_size < sizeof(MessageHeader)) return STATUS_BAD_ARGUMENT;

        if (!message_size)
                return STATUS_OK; /* Okay dude, you wasted everyone's time without sending
                                   * anything. What exactly did you achieve? Here, get a
                                   * STATUS_OK. Have it your way.*/
        if (handle < 0 || handle >= MAX_OBJ_PER_PROCESS) return STATUS_OUT_OF_BOUNDS;

        Process *curr_proc = GetCurrentExecutionThread()->owner;
        Object  *target    = curr_proc->handles.objlist[handle];

        if (!target || target->type != OBJ_PORT || !target->data) return STATUS_NO_ENDPOINT;

        Port *port = target->data;
        if (unlikely(!port->owner)) return STATUS_NO_ENDPOINT;
        if (port->count >= MAX_MESSAGES_IN_PORT) return STATUS_MSGQUEUE_FULL;

        Message *kmsg = &port->msgbuf[port->tail];
        if (unlikely(CopyFromUser(kmsg, m, message_size) != STATUS_OK)) return STATUS_BAD_ARGUMENT;

        /* Mismatch between message_size and payload_length. Need to abort, the message can't be
         * trusted. */
        u32 payload_size = message_size - sizeof(MessageHeader);
        if (kmsg->header.payload_length > payload_size) return STATUS_BAD_ARGUMENT;

        kmsg->header.sender   = curr_proc->pid;
        kmsg->header.reserved = 0;

        int reply_from = kmsg->header.reply_handle;

        /* This message expects a reply. Tell the recipient where to send that reply. */
        if (reply_from >= 0 && reply_from < MAX_OBJ_PER_PROCESS) {
                Object *replyobj = curr_proc->handles.objlist[reply_from];

                if (replyobj && replyobj->type == OBJ_PORT) {
                        HandleTable *target_table = &port->owner->owner->handles;
                        int          new_hdl      = -1;

                        /* Check if the process already has a handle to this port to avoid
                         * duplication. */
                        for (int i = 0; i < MAX_OBJ_PER_PROCESS - 1; i++) {
                                /* If the server already has a handle to this process, no need to
                                 * allocate a new one - Just reuse the existing one. */
                                if ((target_table->valid_bitmap & (1 << i)) &&
                                    target_table->objlist[i] == replyobj) {
                                        new_hdl = i;
                                        break;
                                }
                        }

                        /* If we didn't find an existing one, then we must create a new object to
                         * talk to */
                        if (new_hdl == -1) new_hdl = AppendToHandleTable(target_table, replyobj);

                        kmsg->header.reply_handle = (new_hdl >= 0) ? new_hdl : -1;
                }
        } else
                kmsg->header.reply_handle = -1;

        /* Something I got from Windows NT: Donate the quantum of the current thread to the target
         * thread (port handler, in our case). This allows the recipient to get more time to process
         * the message. But don't clear the current thread's quantum, as it may want to handle the
         * reply later or not even expect a reply, because it has to do other things. IPC is
         * asynchronous when it comes to sending for DragonWare. */
        port->owner->quantum += GetCurrentExecutionThread()->quantum;
        port->tail = (port->tail + 1) % MAX_MESSAGES_IN_PORT;
        port->count++;

        WakeThread(port->owner);
        return STATUS_OK;
}

[[gnu::hot]]
Status _DWIPCReceive(int handle, Message *msave) {
        if (handle < 0 || handle >= MAX_OBJ_PER_PROCESS) return STATUS_OUT_OF_BOUNDS;

        Process *curr_proc = GetCurrentExecutionThread()->owner;
        Object  *obj       = curr_proc->handles.objlist[handle];
        if (!obj || obj->type != OBJ_PORT || !obj->data) return STATUS_NO_ENDPOINT;

        Port *port = (Port *)obj->data;

        /* Block if there are no messages */
        while (!port->count) {
                BlockThread(GetCurrentExecutionThread());
                ScheduleNext();
        };

        Message *to_read = &port->msgbuf[port->head];
        port->head       = (port->head + 1) % MAX_MESSAGES_IN_PORT;
        Status result    = STATUS_OK;
        if (unlikely(CopyToUser(msave, to_read,
                                sizeof(MessageHeader) + to_read->header.payload_length) !=
                     STATUS_OK))
                result = STATUS_BAD;

        port->count--;
        return result;
}
