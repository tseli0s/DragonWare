/**********************************************************************
 * FILE: pit.c
 * PURPOSE: PIT timer support implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "timer/pit.h"

#include <ioport.h>
#include <ktypes.h>
#include <mmutils.h>

#include "cpu/irq.h"

#define TARGET_HZ  (50)

#define PIT_CH0    (0x40)
#define PIT_CMD    (0x43)
#define PIT_FREQ   (1193182)

#define MAX_TIMERS (16)

static volatile u32         ticks              = 0;
static Timer                timers[MAX_TIMERS] = {0};
static TickCallbackFunction tcf[MAX_TIMERS]    = {0};
static Size                 timers_index       = 0;
static Size                 tcf_index          = 0;

static void PITInit(u32 freq) {
        u16 divisor = (u16)(PIT_FREQ / freq);

        outb(PIT_CMD, 0x36);
        outb(PIT_CH0, (Byte)(divisor & 0xFF));
        outb(PIT_CH0, (Byte)((divisor >> 8) & 0xFF));
}

[[gnu::hot]]
static void PITCallback(IRQRegisters *r) {
        (void)r;
        ticks++;
        for (unsigned int i = 0; i < tcf_index; i++) {
                if (tcf[i]) tcf[i](ticks);
        }
        if ((ticks % TARGET_HZ) == 0) {
                /* One second passed, update all timers */
                for (unsigned int i = 0; i < timers_index; i++) {
                        timers[i].seconds -= (1 * TARGET_HZ);
                        if (timers[i].seconds <= 0) {
                                timers[i].callback();
                                /* Skip the current entry so that it doesn't run again */
                                memcpy(timers + (i * sizeof(Timer)),
                                       timers + ((i + 1) * sizeof(Timer)), sizeof(Timer));
                        }
                }
        }
}

void StartPITTimer(void) {
        PITInit(TARGET_HZ);
        RegisterIRQHandler(0, PITCallback);
}

void Sleep(u64 seconds) {
        u64 target = ticks + ((u64)seconds * TARGET_HZ);
        while (ticks < target) __asm__ volatile("pause");
}

Timer AddCountdownTimer(int seconds, TimerCallbackFunction f) {
        if (timers_index >= MAX_TIMERS) return (Timer){.seconds = 0, .callback = NullPointer};
        Timer t              = {.seconds = seconds * TARGET_HZ, .callback = f};
        timers[timers_index] = t;
        timers_index++;
        return t;
}

int AddTickCallbackFunction(TickCallbackFunction f) {
        if (tcf_index >= MAX_TIMERS) return -1;
        tcf[tcf_index] = f;
        tcf_index++;
        return tcf_index - 1;
}

void RemoveTickCallbackFunction(int index) {
        if (index >= MAX_TIMERS) return;
        tcf[index] = NullPointer;
}
