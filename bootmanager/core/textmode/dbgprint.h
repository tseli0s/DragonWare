/**********************************************************************
 * FILE: dbgprint.h
 * PURPOSE: VGA text mode implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/**
 * @brief Initializes the serial port to accept new messages
 * @sa DebugPrint
 */
void InitDebugPrint(void);

/**
 * @brief Prints a message to the serial output for debugging purposes.
 * @note For debugging purposes. Normal bootloader layout/visuals are drawn using @ref vgatext.c
 * @param msg A format string that once formatted will be printed to the serial port COM1
 */
void DebugPrint(const char *msg, ...);
