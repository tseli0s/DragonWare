/**********************************************************************
 * FILE: ipc86.h
 * PURPOSE: Microkernel API system call exports for userspace applications
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef _IPC86_H
#define _IPC86_H             1

#define DEFAULT_MESSAGE_SIZE (256)

#include "cppsupport.h"
#include "kerneltypes.h"

DW_BEGIN_DECLS

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
 * @brief A header embedded in every IPC message between processes,
 * or kernel to process messages.
 * @details This header is always present in every @ref Message sent and received
 * between processes. It contains information about the message contents, like the size of
 * the payload, a reply handle and the protocol used.
 * @since v0.0.2
 * @sa Message
 */
typedef struct [[gnu::packed]] _MessageHeader {
        u32 msgid;        /** << ID of the message. Used to match one request to one unique reply
                                     from the receiver. */
        int reply_handle; /** << Handle to the reply port. The kernel automatically
                             translates that between processes. If -1, no reply is expected
                             for the message */
        u32 sender;       /** << ID of the sender process (The process that sent this message).
                             This   is set by the kernel only. */
        u16 type;         /** <<  Type of the message. This is to be determined by the two processes
                             according to their protocols. */
        u16 protocol;     /** << If a process supports different IPC protocols, one may be
                             specified here. */
        u32 payload_length; /** << Size of the @ref payload used. More bytes may not be read
                               off of @ref payload. */
        u32 reserved;       /** << Reserved. Should be 0.  */

} MessageHeader;

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
        MessageHeader header;
        union {
                Byte raw[MESSAGE_BUFFER_SIZE]; /** << Raw data left up for interpetation by the
                                                  processes. */
                /* TODO: Add more fields here depending on how much the kernel wants to involve
                 * itself */
        } payload;
} Message;

DW_END_DECLS

#endif /* _IPC86_H */
