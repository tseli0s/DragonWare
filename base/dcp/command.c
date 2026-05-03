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

/* Helper only because I am too lazy */
#define STREQUAL(_a, _b) (strcmp(_a, _b) == 0)

static inline Bool hasspace(const char *str) { return (strchr(str, ' ') != NullPointer); }

static void PrintHelp(void) {
        puts("List of builtin commands:");
        puts("* 'help' - Prints this helper");
        puts("* 'identify' - Prints version and build information about DragonWare");
        puts("* 'exit' - Terminates this process and ends the session");
}

/* TODO: This is also implemented as a separate program (see base/ver/ver.c) but DragonWare can't
 * load arbitrary programs yet */
static void IdentifySystem(void) {
        SystemIdentify sysdata;
        _DWSystemIdentify(&sysdata);

        printf("* %s version %u.%u.%u-%s. This build has build ID number %u.\n\n", sysdata.name,
               sysdata.major, sysdata.minor, sysdata.patch, sysdata.tag, sysdata.build_id);
        puts("* DragonWare is free software, distributed and provided to you under the\n"
             "terms of the GNU General Public License, version 3. This operating system comes "
             "with ABSOLUTELY NO WARRANTY.\nSee the GNU General Public License for more "
             "details.\n\n");
}

void HandleCommandBuffer(const char *cmd) {
        if (STREQUAL(cmd, "help")) {
                if (hasspace(cmd))
                        puts("Bad amount of arguments\n");
                else
                        PrintHelp();
        } else if (STREQUAL(cmd, "identify")) {
                if (hasspace(cmd))
                        puts("Bad amount of arguments\n");
                else
                        IdentifySystem();
        } else if (STREQUAL(cmd, "exit")) {
                exit(0);
        } else {
                puts("Bad command. Check if you typed the command correctly and try again.\n");
        }
}
