/**********************************************************************
 * FILE: ipc_console.h
 * PURPOSE: Console IPC helpers
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <object.h>

/* Prints a single message in the console. A helper, until printf() is implemented. */
void PrintMessageToConsole(Handle console_handle, char *msg);
