/**********************************************************************
 * FILE: systime.h
 * PURPOSE: System time support (Based on CMOS values)
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/*
 * Simple time/date API I wrote while bored.
 * Tldr just reads whatever the CMOS battery has as its
 * time/date and then constructs the SystemTime struct with
 * that data.
 *
 * Speaking of, at some point we should have a way for the kernel to adjust its
 * date and time from userspace. Probably a syscall?
 */

#include <ktypes.h>

/**
 * @brief Represents the current system time.
 * @details The fields are stored as Bytes and correspond to the values read from the
 * CMOS real-time clock (RTC). The year value is stored in an implementation-defined
 * format (commonly an offset from 2000 or 1900 depending on BIOS/CMOS settings).
 */
typedef struct _SystemTime {
        Byte year;    /**< Year value from RTC (implementation-defined offset). */
        Byte month;   /**< Month value (1–12). */
        Byte day;     /**< Day of month (1–31). */
        Byte hour;    /**< Hour of day (0–23). */
        Byte minutes; /**< Minutes (0–59). */
        Byte seconds; /**< Seconds (0–59). */
} SystemTime;

/**
 * @brief Read the current system time from the CMOS RTC.
 * @returns A populated @ref SystemTime structure with the current time.
 * @note This function reads and converts RTC values to binary and 24-hour format
 * if necessary.
 */
SystemTime GetSystemTime(void);
