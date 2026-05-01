/**********************************************************************
 * FILE: irq.c
 * PURPOSE: IRQ setup and handling code
 * PROJECT: DragonWare Kernel
 * DATE: 09-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "irq.h"

#include <ioport.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "interrupts.h"
#include "irq.h"
#include "pic.h"
#include "sched/schedule.h"
#include "task/message.h"
#include "task/task.h"

/**
 * @brief Sends an End Of Interrupt signal to the PIC controller. This should be called every time
 * an interrupt is handled.
 * @param[in] irq_no The number of the IRQ that was serviced. Should be between 0 and 15.
 */
[[gnu::hot, gnu::always_inline]]
static inline void _SendEOI(u32 irq_no) {
        if (irq_no >= 8) outb(0xA0, 0x20);
        outb(0x20, 0x20);
}

extern volatile int NeedsResched;

static IRQHandler      IRQHandlerCallbacks[MAX_IRQ] = {0};
static IRQRelayHandler IRQRelayCallback[MAX_IRQ]    = {0};

void RegisterIRQHandler(int which, IRQHandler handler) {
        /* assume that the handler should be removed */
        if (!handler) {
                DisableIRQ((u32)which);
                return;
        }
        if (inrange(which, 0, MAX_IRQ)) {
                IRQHandlerCallbacks[which] = handler;
                EnableIRQ((u32)which);
        }
}

void RegisterIRQSubscriber(unsigned int which, int port_handle) {
        /* IRQ 0 is explicitly reserved, it is the timer that runs the whole system. */
        if (unlikely(!inrange(which, 1, MAX_IRQ - 1))) return;
        Thread* current = GetCurrentExecutionThread();
        if (unlikely(!current)) {
                /* Assuming it's a kernel bug here */
                LogMessage(
                        LOG_ERROR,
                        "KERNEL BUG: RegisterIRQSubscriber called without an active user thread!");
                return;
        }

        IRQRelayCallback[which] = (IRQRelayHandler){
                .handler = current, .port_handle = port_handle, .is_active = true};
        EnableIRQ(which);
}

[[gnu::hot]]
void AcknowledgeUserIRQ(unsigned int which, int port_handle) {
        if (unlikely(!inrange(which, 1, MAX_IRQ - 1))) return;
        IRQRelayHandler handler = IRQRelayCallback[which];
        if (!handler.is_active) return;
        if (handler.handler != GetCurrentExecutionThread()) return;
        if (handler.port_handle != port_handle) return;

        EnableIRQ(which);
}

[[gnu::hot]]
void IRQHandlerCallback(InterruptStackFrame* r) {
        u32 irq = r->int_no - IRQ_BASE; /* IRQs come right after all the 32 (0x20) IDT entries */

        /*
         * Send EOI before handling the interrupt, as IRQs might preempt the current task and switch
         * to another. NOTE: Normally an EOI allows more interrupts to come. However we disabled
         * interrupts when we entered the IRQ handling code. They will be reenabled when iret pops
         * the eflags that have the IF bit set automatically, which is the only reason this code
         * works. */
        DisableIRQ(irq);
        _SendEOI(irq);

        if (likely(irq < MAX_IRQ)) {
                IRQHandler      handler      = IRQHandlerCallbacks[irq];
                IRQRelayHandler user_handler = IRQRelayCallback[irq];

                if (likely(handler != NullPointer)) {
                        handler(r);
                        EnableIRQ(irq);
                        return;
                } else if (user_handler.is_active) {
                        /* I have absolutely no idea why, but setting payload_length to 0 directly
                         * when declaring the struct produced a huge payload length (almost like
                         * underflow, but not exactly, the number I got was 2056952320 or 0x7a9a9a00
                         * in hex). I have absolutely no idea where the problem is, and I don't want
                         * to do a workaround.
                         */
                        Message irqmsg;
                        Thread* target = user_handler.handler;
                        memset(&irqmsg, 0, sizeof(Message));

                        irqmsg.header.sender         = KERNEL_SENDER;
                        irqmsg.header.payload_length = 0;
                        irqmsg.header.reserved       = 0;
                        if (SendMessage(target->owner, user_handler.port_handle, &irqmsg) !=
                            STATUS_OK) {
                                LogMessage(LOG_ERROR,
                                           "Cannot send IRQ relay message to thread with ID %u. "
                                           "Hardware may "
                                           "appear stuck or misbehave.",
                                           user_handler.handler->id);
                                return;
                        }

                        /* Reenabling the IRQ is handled by the IRQ port when an PORT_ACK_IRQ
                         * message is sent, see sys/syscall/object.c Without that, this code
                         * deadlocks the keyboard. */
                        WakeThread(target);
                }
        }
}

void IRQInit(void) {
        ZeroMemory(IRQRelayCallback);
        for (int i = 0; i < MAX_IRQ; i++) IRQHandlerCallbacks[i] = NullPointer;
        InitializePIC();
}
