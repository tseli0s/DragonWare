/**********************************************************************
 * FILE: command.h
 * PURPOSE: Builtin command handling
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <object.h>

/* Handles a prompt submitted by the user. For now, only the builtin commands are supported -
 * Running arbitrary files has not been implemented yet (TODO) */
void HandleCommandBuffer(const char *cmd);
