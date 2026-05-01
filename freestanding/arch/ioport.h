/**********************************************************************
 * FILE: ioport.h
 * PURPOSE: Port I/O instruction implementation for C (outb, inw, ...)
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/*
 * Simple port I/O functions for the CPU based on the instructions like inb, inw, ...
 * Yeah I have absolutely no idea what they're called I just know that they take
 * or give data and they work with the registers. Pretty much the entire kernel
 * needs them, so here they are. */

#include <ktypes.h>
#include <macros.h>

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

[[gnu::always_inline]]
static inline void outw(u16 port, u16 val) {
        __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

[[gnu::always_inline]]
static inline u16 inw(u16 port) {
        u16 ret;
        __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

[[gnu::always_inline]]
static inline void outl(u16 port, u32 val) {
        __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

[[gnu::always_inline]]
static inline u32 inl(u16 port) {
        u32 ret;
        __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}
