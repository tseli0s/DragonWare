/**********************************************************************
 * FILE: irq.h
 * PURPOSE: IRQ constants and definitions
 * PROJECT: DragonWare Kernel
 * DATE: 09-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define MAX_IRQ  (16)   /* Maximum amount of IRQs supported by the PIC. */
#define IRQ_BASE (0x20) /* Every interrupt below that is not an IRQ */

#include <ioport.h>
#include <ktypes.h>
#include <macros.h>

#include "interrupts.h"
#include "task/task.h"

/**
 * @brief An IRQ relay handler descriptor.
 * @details IRQ relaying is how DragonWare solves the problem of IRQs being an entirely kernel thing
 * but all drivers being implemented as unprivileged userspace servers. When an IRQ arrives, the
 * kernel will check if there's a userspace thread to handle the IRQ, and notify it that the IRQ
 * arrived.
 *
 * DragonWare's kernel uses a very specific message format to relay IRQs. Most importantly, the
 * sender is marked as PID 0, which is the kernel itself (All processes start from PID 1).
 */
typedef struct _IRQRelayHandler {
        Thread* handler;     /* Which thread to wake up for the IRQ */
        int     port_handle; /* To which port should we send the message to */
        Bool    is_active;   /* If true, this IRQ is claimed by a userspace server */
} IRQRelayHandler;

/**
 * @brief A kernel mode IRQ callback routine.
 * @note This is only used for devices which cannot be used from userspace (like the PIT timer).
 * Other IRQs must be handled by userspace servers entirely and the kernel should only notify them
 * that an IRQ arrived.
 */
typedef void (*IRQHandler)(InterruptStackFrame*);

/**
 * @brief Registers a kernel mode IRQ handler that will be invoked when @p which IRQ is fired.
 * @param which Which IRQ is this handler handling. Must be between 0 and 15.
 * @param[in] handler The handler to run. If NullPointer, the handler is removed instead and the IRQ
 * is disabled.
 * @sa RegisterIRQSubscriber
 */
void RegisterIRQHandler(int which, IRQHandler handler);

/**
 * @brief Registers the currently running thread as the IRQ handler that will be invoked
 * whenever @p which IRQ is dispatched from the hardware.
 * @param which Which IRQ is this subscriber handling. Must be between 1-15.
 * @param port_handle Handle to the port where the messages will be dispatched when an IRQ fires.
 * @note Can only be used when there's an active thread to register as the IRQ handler, and that
 * thread is in user mode.
 * @warning This should only be called from system calls, with interrupts disabled.
 */
void RegisterIRQSubscriber(unsigned int which, int port_handle);

/**
 * @brief Acknowledge an IRQ that was rerouted to userspace.
 * @param which Which IRQ to acknowledge. Must be between 1-15.
 * @param port_handle Port handle to the port that is used for IRQ relaying. Must be between
 * 0-MAX_OBJ_PER_PROCESS-1.
 */
void AcknowledgeUserIRQ(unsigned int which, int port_handle);

/** @brief Initializes IRQ handling in the kernel */
void IRQInit(void);

/** @brief Invoked whenever an IRQ fires to choose the correct handler and send an EOI signal. */
void IRQHandlerCallback(InterruptStackFrame* r);
