/**********************************************************************
 * FILE: tui.h
 * PURPOSE: Text mode user interface for the bootloader
 * PROJECT: DragonWare Boot Manager
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/**
 * @brief Draw the user interface on the screen by writing characters at their expected position.
 * @note This should only be called when the UI has changed. Otherwise it is expensive to run this.
 */
void DrawUserInterface(void);
