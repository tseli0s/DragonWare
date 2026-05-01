/**********************************************************************
 * FILE: io.h
 * PURPOSE: Port I/O helpers for the PS/2 keyboard driver
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kerneltypes.h>

[[gnu::always_inline]]
static inline void outb(u16 port, Byte val) {
        __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

[[gnu::always_inline]]
static inline Byte inb(u16 port) {
        Byte ret;
        __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}
