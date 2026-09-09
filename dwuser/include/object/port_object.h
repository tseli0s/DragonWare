/**********************************************************************
 * FILE: port.h
 * PURPOSE: Port object helpers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <object.h>

/**
 * @brief Create a new port for sending and receiving IPC messages.
 * @param[in] name Name of the port. If @ref NullPointer, this port is local to the process. If it's
 * a string, the kernel will make this a global port, and allow other processes to discover it and
 * send messages to it.
 * @returns Handle to the newly created port, or -1 on failure.
 */
[[nodiscard(
        "Returned handle is a handle that references the port object and must not be discarded.")]]
Handle CreatePort(const char *name);

/**
 * @brief Open a global port and return a handle to it for sending messages.
 * @param[in] name Name of the port. Must not be @ref NullPointer.
 * @returns Handle to the port, or -1 on failure (eg. Port not found).
 */
[[gnu::nonnull, nodiscard("Returned handle is a handle that references the port object and must "
                          "not be discarded.")]]
Handle OpenPort(const char *name);

/**
 * @brief Synchronous remote address space call.
 * @details This function is similar to a @b Remote @b Procedure @b Call by making a request and
 * waiting on a reply to/from a different address space (ie. A process). Specifically:
 * 1. A message contained in @p msgbuf is sent to the handle of the port @p send
 * 2. @ref ReceiveMessage is invoked on @p recv blocking until a message arrives.
 * 3. When a message does arrive on @p recv, it is copied back on @p msgbuf and the function returns
 * the status code.
 * @note This function only works if there are no messages already in @p recv. Draining the port
 * pointed to by @p recv is important.
 * @param[in,out] msgbuf Pointer to copy the message from and store the incoming message to. Must
 * not be a @ref NullPointer
 * @param send Handle to the port to send the message to.
 * @param recv Handle to the port to receive the reply from.
 * @returns STATUS_OK on success, otherwise the result of @ref SendMessage if sending failed or @ref
 * ReceiveMessage if (unlikely) receiving failed.
 */
[[nodiscard(
        "Sending or receiving messages may fail (eg. Bad handle) and the caller must inspect the "
        "return value.")]]
Status IPCCall(Message *msgbuf, Handle send, Handle recv);

/**
 * @brief Wrapper for invoking @p port with the @ref PORT_BIND_IRQ opcode.
 * @param port Port to bind the incoming IRQs to.
 * @param irq IRQ to bind to. As of @version v0.0.2, this must be 1-15.
 * @returns The result of @ref InvokeObject
 * @note A separate @ref IRQBindingDescriptor is allocated every time this function is called.
 */
static inline Status BindIRQ(Handle port, u32 irq) {
        IRQBindingDescriptor descr = {.irq_no = irq, .reserved = 0};
        return InvokeObject(port, PORT_BIND_IRQ, &descr);
}

/**
 * @brief Wrapper for invoking @p port with the @ref PORT_ACK_IRQ opcode.
 * @param port Port to acknowledge the IRQ for.
 * @param irq IRQ to acknowledge (Multiple IRQs may arrive at a port). As of @version v0.0.2, this
 * must be 1-15.
 * @returns The result of @ref InvokeObject
 * @note A separate @ref IRQBindingDescriptor is allocated every time this function is called.
 */
static inline Status AcknowledgeIRQ(Handle port, u32 irq) {
        IRQBindingDescriptor descr = {.irq_no = irq, .reserved = 0};
        return InvokeObject(port, PORT_ACK_IRQ, &descr);
}
