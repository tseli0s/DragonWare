/**********************************************************************
 * FILE: systime.c
 * PURPOSE: System time support (Based on CMOS values)
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "systime.h"

#include <ioport.h>

typedef enum _CMOSPort { CMOS_OUT_PORT = 0x70, CMOS_IN_PORT = 0x71 } CMOSPort;

static SystemTime global_time;

static inline Byte ReadFromCMOSRegister(Byte reg) {
        outb(CMOS_OUT_PORT, reg);
        return inb(CMOS_IN_PORT);
}

static inline Byte BCDToBinary(Byte v) {
        /*
         * Let me translate what you're seeing:
         * - Take the lower four bits of v as a number (So a number between 0-9, 10-15 are invalid
         * in BCD). That's the ones digit in BCD.
         * - Now take the upper four bits of v to get the tens digit in BCD.
         * - If you find the tens and add the ones, you get a regular number.
         * */
        return (Byte)((v & 0x0F) + ((v >> 4) * 10));
}

static void SystemTimeInit(void) {
        Byte reg_b;
        Byte sec, min, hour, day, mon, year;
        Bool is_24h;
        Bool is_binary;

        /* Reading from the CMOS while it's updating is undefined behaviour */
        while (ReadFromCMOSRegister(0x0A) & 0x80);

        sec   = ReadFromCMOSRegister(0x00);
        min   = ReadFromCMOSRegister(0x02);
        hour  = ReadFromCMOSRegister(0x04);
        day   = ReadFromCMOSRegister(0x07);
        mon   = ReadFromCMOSRegister(0x08);
        year  = ReadFromCMOSRegister(0x09);
        reg_b = ReadFromCMOSRegister(0x0B);

        is_24h    = reg_b & 0x02;
        is_binary = reg_b & 0x04;

        if (!is_binary) {
                /* We must convert to binary here */
                sec  = BCDToBinary(sec);
                min  = BCDToBinary(min);
                day  = BCDToBinary(day);
                mon  = BCDToBinary(mon);
                year = BCDToBinary(year);
        }

        if (!is_24h) {
                Bool pm = hour & 0x80;
                hour &= 0x7F;

                if (!is_binary) hour = BCDToBinary(hour);
                if (pm && hour != 12)
                        hour += 12;
                else if (!pm && hour == 12)
                        hour = 0;
        } else {
                if (!is_binary) hour = BCDToBinary(hour);
        }

        global_time.seconds = sec;
        global_time.minutes = min;
        global_time.hour    = hour;
        global_time.day     = day;
        global_time.month   = mon;
        global_time.year    = year;
}

SystemTime GetSystemTime(void) {
        SystemTimeInit();
        return global_time;
}
