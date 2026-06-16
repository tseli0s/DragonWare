/**********************************************************************
 * FILE: console.h
 * PURPOSE: Framebuffer protocol definitions
 * PROJECT: DragonWare Base System
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

#pragma once

#include <ipc86.h>

#define PIXEL_INDEX(_x, _y, _fbwidth) (_y * _fbwidth + _x)

void HandleConsoleClientRequest(Message *m);
