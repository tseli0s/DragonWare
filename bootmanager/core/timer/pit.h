/**********************************************************************
 * FILE: pit.h
 * PURPOSE: PIT timer support implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

typedef void (*TimerCallbackFunction)(void);
typedef void (*TickCallbackFunction)(u32 current_ticks);

typedef struct _Timer {
        i32                   seconds;
        TimerCallbackFunction callback;
} Timer;

/* Starts the PIT timer and installs the necessary interrupts */
void StartPITTimer(void);

/* Temporarily stops the machine for seconds given */
void Sleep(u64 seconds);

/* Creates a countdown timer and returns a structure that is described every time */
Timer AddCountdownTimer(int seconds, TimerCallbackFunction f);

/* Add a function that will be called every time the timer fires. Returns the index of the added
 * callback (so that it can be removed later)*/
int AddTickCallbackFunction(TickCallbackFunction f);

/* Removes a callback function from being called at the given index, usually returned from
 * AddTickCallback function */
void RemoveTickCallbackFunction(int index);
