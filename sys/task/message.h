/**********************************************************************
 * FILE: message.h
 * PURPOSE: IPC message format definition
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/* For larger messages I'll implement some sort of memory sharing or something idk */

/**
 * @brief The default size of arbitrary data within a message (Not set by the kernel in any way)
 * @note All messages have this buffer stored internally, even if it remains unused.
 */
#define MESSAGE_BUFFER_SIZE (256)

/**
 * @brief Sender PID of the kernel.
 * @details This is the PID set in the header if the message comes from the kernel and not from
 * another process. 0 is reserved, and all processes start from @ref ProcessID 1 or @ref ThreadID 1.
 */
#define KERNEL_SENDER       (0)

/**
 * @brief A single IPC message sent between two processes (Or the kernel and a process, in some
 * cases.)
 * @details In DragonWare, IPC messages are the primary means of functionality of the operating
 * system. Messages are used to invoke a service of the operating system. For example, a message
 * sent to the filesystem server is used to fetch a file from the disk. In turn, the filesystem
 * server sends a message to the disk driver to read the bytes where the file is stored into a piece
 * of memory.
 */
typedef struct [[gnu::packed]] _Message {
        struct [[gnu::packed]] {
                u32 msgid; /** << ID of the message. Used to match one request to one unique reply
                              from the receiver. */
                int reply_handle; /** << Handle to the reply port. The kernel automatically
                                     translates that between processes. If -1, no reply is expected
                                     for the message */
                u32 sender; /** << ID of the sender process (The process that sent this message).
                               This   is set by the kernel only. */
                u16 type; /** <<  Type of the message. This is to be determined by the two processes
                             according to their protocols. */
                u16 protocol;       /** << If a process supports different IPC protocols, one may be
                                       specified here. */
                u32 payload_length; /** << Size of the @ref payload used. More bytes may not be read
                                       off of @ref payload. */
                u32 reserved;       /** << Reserved. Should be 0.  */
        } header;
        union {
                Byte raw[MESSAGE_BUFFER_SIZE]; /** << Raw data left up for interpetation by the
                                                  processes. */
                /* TODO: Add more fields here depending on how much the kernel wants to involve
                 * itself */
        } payload;
} Message;

/* Forward declarations to avoid circular includes */
typedef struct _Process      Process;
typedef struct _Port         Port;
typedef struct _MessageQueue MessageQueue;

/**
 * @brief Enqueues a new message to be handled by the given @p port owner.
 * @note @p new_message must have the message copied first inside MessageQueue::m!
 * @param[in] port The port to enqueue the message to. Must not be NullPointer.
 * @param[in] new_message The message queue to push back in the port's message queue. Must not be
 * NullPointer.
 */
void EnqueueMessage(Port *port, MessageQueue *new_message);

/**
 * @brief Sends a message from the kernel to the thread @p thread
 * @param[in] process The process to send the message to.
 * @param[in] handle Handle to the object to enqueue the message to. Must be >=0.
 * @param[in] msg The message to send. Must not be NullPointer.
 * @returns STATUS_OK if the message was sent, STATUS_BAD if the sender is not valid.
 */
[[gnu::nonnull, gnu::hot]]
Status SendMessage(Process *process, int handle, Message *msg);
