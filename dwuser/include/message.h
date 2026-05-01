/**********************************************************************
 * FILE: message.h
 * PURPOSE: IPC messaging wrappers for DragonWare applications
 * PROJECT: DragonWare User Library
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "ipc86.h"
#include "kerneltypes.h"
#include "object.h"

/**
 * @brief _DWIPCSend system call (#5) wrapper
 * @details This routine is going to store a message and wake the owner of the target handle to
 * handle it. It is explicitly asynchronous and does not guarantee that the message will be
 * received.
 * @param[in] handle The handle to submit the message to. See @ref _DWCreateObject
 * @param[in] m The message to copy. Cannot be a @ref NullPointer.
 * @param[in] message_size Message size to copy. Must be at least the size of the header.
 * @returns @ref STATUS_OK if the message was succesfully sent, @ref STATUS_NO_ENDPOINT if the
 * target @p pid is not present, other @ref Status codes for other kinds of failures.
 */
[[gnu::nonnull]]
Status SendMessage(Handle handle, Message *m, Size message_size);

/**
 * @brief _DWIPCReceive system call (#6) wrapper.
 * @details This routine pops a message from the process' internal queue, copies it in @p msave if
 * and only if the message was sent in the matching @p handle. The process is automatically blocked
 * if there are no messages to wait a busy loop.
 * @param handle The handle to receive messages from. Must be between 0 and MAX_OBJ_PER_PROCESS-1
 * (Kernel define, usually 32)
 * @param[out] msave A pointer to save the message to. Must not be a NullPointer.
 * @return STATUS_RETRY if there are no messages queued. STATUS_OK if a message was found to be
 * returned. STATUS_BAD if copying the message failed.
 */
[[gnu::nonnull]]
Status ReceiveMessage(Handle handle, Message *msave);
