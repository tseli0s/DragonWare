/**********************************************************************
 * FILE: ver.c
 * PURPOSE: Utility to output version information about DragonWare at runtime
 * PROJECT: DragonWare Base System
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <stdio.h>

int main(void) {
        SystemIdentify sysdata = {0};
        _DWSystemIdentify(&sysdata);
        /* Zero length strings -> Assuming there's nothing to read */
        if (!sysdata.name[0] || !sysdata.tag[0]) {
                puts("syscall failed, no data available");
                return -1;
        }

        printf("* System Name: %s (%s)\n", (char*)sysdata.name, (char*)sysdata.tag);
        printf("* System Version: %d.%d.%d\n", sysdata.major, sysdata.minor, sysdata.patch);
        printf("* Kernel Build ID: %llu\n", sysdata.build_id);

        return 0;
}
