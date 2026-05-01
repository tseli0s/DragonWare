/**********************************************************************
 * FILE: timer.c
 * PURPOSE: PIT timer driver, used for preemption and time tracking
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "timer.h"

#include <ioport.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>

#include "ddk/ia32/interrupts.h"
#include "ddk/ia32/irq.h"
#include "iomgr/class.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "sched/schedule.h"
#include "task/task.h"

#define TARGET_HZ (100)
static volatile u64 ticks = 0;

extern volatile int NeedsResched;

static void PITInit(u32 freq) {
        u16 divisor = (u16)(PIT_FREQ / freq);

        outb(PIT_CMD, 0x36);
        outb(PIT_CH0, (Byte)(divisor & 0xFF));
        outb(PIT_CH0, (Byte)((divisor >> 8) & 0xFF));
}

[[gnu::hot]]
static void PITCallback(InterruptStackFrame *r) {
        UnusedParameter(r);
        ticks++;
        Thread *curr = GetCurrentExecutionThread();
        if (curr && curr->quantum-- == 0) NeedsResched = 1;
}

void StartSystemTimer(void) {
        PITInit(TARGET_HZ);
        RegisterIRQHandler(0, PITCallback);
        LogMessage(LOG_DEBUG, "Starting hardware-based system timer");

        AddDevice(NullPointer,
                  MakeDeviceNode("PIT Timer", P_USER | P_DIRECT_ACCESS, DEVCLASS_UNKNOWN));
}

void Sleep(u32 seconds) {
        u64 target = ticks + ((u64)seconds * TARGET_HZ);
        while (ticks < target) __asm__ volatile("pause");
}

u64 GetTicksSinceBoot(void) { return ticks; }
