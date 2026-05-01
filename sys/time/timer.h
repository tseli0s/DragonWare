/**********************************************************************
 * FILE: timer.h
 * PURPOSE: PIT timer driver, used for preemption and time tracking
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
/**
 * @brief I/O port address for PIT channel 0 data register.
 * @details Channel 0 is typically used for system timer interrupts.
 */
#define PIT_CH0  (0x40)

/**
 * @brief I/O port address for the PIT command register.
 * @details The command register is used to configure the operating mode and access
 * mode for the PIT channels.
 */
#define PIT_CMD  (0x43)

/**
 * @brief The base input clock frequency of the PIT.
 * @details This value is fixed for the standard PC PIT and is used to calculate
 * timer reload values.
 * @warning You REALLY don't want to change this #define.
 */
#define PIT_FREQ (1193182)

/**
 * @brief Start the system timer.
 * @details Currently, this initializes and starts the Programmable Interval Timer (PIT) only.
 * In future revisions, the timer implementation will be replaced by an abstraction layer
 * that chooses between PIT and HPET based on available hardware.
 */
void StartSystemTimer(void);

/**
 * @brief Suspend execution for a given number of seconds.
 * @param seconds Number of seconds to sleep.
 * @note This function uses the system timer (PIT) and blocks the current thread of execution.
 */
void Sleep(u32 seconds);

/**
 * @brief Get the amount of clock ticks since boot.
 * @note The clock is usually configured at 1000Hz.
 * @warning A few ticks are lost during early bootstrap, when the system timer is not active yet.
 * This is a couple milliseconds.
 * @return Ticks since boot as an unsigned 64 bit integer.
 */
u64 GetTicksSinceBoot(void);
