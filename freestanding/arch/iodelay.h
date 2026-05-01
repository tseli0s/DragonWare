/**********************************************************************
 * FILE: ioport.h
 * PURPOSE: Port I/O instruction implementation for C (outb, inw, ...)
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "ioport.h"

/** @brief Small delay to further delay a fast CPU from finishing with a timeout decrement really
 * fast, by reading from (slow) ports. Intended to be called in loops where a counter is used as
 * timeout, as processors may finish the counter increment really fast and not reflect on the
 * hardware speed. */
static inline void IODelay(void) { inb(0x80); }
