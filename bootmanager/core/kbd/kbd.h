/**********************************************************************
 * FILE: kbd.h
 * PURPOSE: PS/2 Keyboard support implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* Remember the arrow keys are extended keys, we must first see if 0xe0 was sent */
#define SCANCODE_EXTENDED_COMING (0xE0)
#define SCANCODE_UP_KEY          (0x48)
#define SCANCODE_DOWN_KEY        (0x50)

/* Simple PS/2 keyboard support, largely copied from DragonWare's kernel driver */

void InitPS2Keyboard(void);
