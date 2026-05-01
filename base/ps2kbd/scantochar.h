/**********************************************************************
 * FILE: scantochar.h
 * PURPOSE: PS/2 keyboard scancode to ASCII character conversion
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/
#pragma once

#include <kerneltypes.h>

static inline char ScancodeToCharacter(const char *ascii_table, u8 scancode) {
        if (scancode & 0x80) return 0; /* Key release, not meant to be printed. */
        return ascii_table[scancode];
}
