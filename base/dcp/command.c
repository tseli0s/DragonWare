/**********************************************************************
 * FILE: command.c
 * PURPOSE: Builtin command handling
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "command.h"

#include <kerneltypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ipc_console.h"

/* Helper only because I am too lazy */
#define STREQUAL(_a, _b) (strcmp(_a, _b) == 0)

static inline Bool hasspace(const char *str) { return (strchr(str, ' ') != NullPointer); }

static void PrintHelp(Handle consport) {
        PrintMessageToConsole(consport, "List of builtin commands: \n");
        PrintMessageToConsole(consport, "* 'help' - Prints this helper\n");
        PrintMessageToConsole(
                consport, "* 'identify' - Prints version and build information about DragonWare\n");
        PrintMessageToConsole(consport,
                              "* 'exit' - Terminates this process and ends the session\n");
}

/* TODO: This is also implemented as a separate program (see base/ver/ver.c) but DragonWare can't
 * load arbitrary programs yet */
static void IdentifySystem(Handle consport) {
        SystemIdentify sysdata;
        _DWSystemIdentify(&sysdata);

        char buffer[128];
        snprintf(buffer, sizeof(buffer),
                 "* %s version %u.%u.%u-%s. This build has build ID number %u.\n\n", sysdata.name,
                 sysdata.major, sysdata.minor, sysdata.patch, sysdata.tag, sysdata.build_id);
        PrintMessageToConsole(consport, buffer);
        PrintMessageToConsole(
                consport,
                "* DragonWare is free software, distributed and provided to you under the\n"
                "terms of the GNU General Public License, version 3. This operating system comes "
                "with ABSOLUTELY NO WARRANTY.\nSee the GNU General Public License for more "
                "details.\n\n");
}

void HandleCommandBuffer(Handle consport, const char *cmd) {
        if (STREQUAL(cmd, "help")) {
                if (hasspace(cmd))
                        PrintMessageToConsole(consport, "Bad amount of arguments\n");
                else
                        PrintHelp(consport);
        } else if (STREQUAL(cmd, "identify")) {
                if (hasspace(cmd))
                        PrintMessageToConsole(consport, "Bad amount of arguments\n");
                else
                        IdentifySystem(consport);
        } else if (STREQUAL(cmd, "exit")) {
                exit(0);
        } else {
                PrintMessageToConsole(
                        consport,
                        "Bad command. Check if you typed the command correctly and try again.\n");
        }
}
