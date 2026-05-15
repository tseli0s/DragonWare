/**********************************************************************
 * FILE: main.c
 * PURPOSE: Kernel-wide multiboot information management
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "bootinfo.h"

#include <kstring.h>
#include <macros.h>
#include <mmutils.h>

#include "ktypes.h"
#include "log.h"
#include "vendor/multiboot.h"

#define MAX_BOOT_MODULES (32)
#define MAX_CMDLINE      (512)

static Multiboot       _bootinfo                      = {0};
static MultibootModule boot_modules[MAX_BOOT_MODULES] = {0};
static unsigned int    n_boot_modules                 = 0;

/* Since this is filled very early at boot, we cannot allocate memory at runtime - We have to keep
 * it in the executable and copy it here. */
static char boot_cmdline[512];

void RegisterBootInformation(Multiboot *bootinfo) {
        ZeroMemory(boot_modules);
        ZeroMemory(boot_cmdline);
        memcpy(&_bootinfo, bootinfo, sizeof(Multiboot));

        if (bootinfo->flags & MULTIBOOT_CMDLINE)
                strncpy(boot_cmdline, (char *)bootinfo->cmdline, MAX_CMDLINE);
}

Multiboot *GetBootInformationStructure(void) { return &_bootinfo; }

Status GetBootModuleAt(unsigned int index, MultibootModule *dest) {
        if (index >= n_boot_modules) return STATUS_OUT_OF_BOUNDS;
        *dest = boot_modules[index];
        return STATUS_OK;
}

void RegisterBootModules(void) {
        n_boot_modules = _bootinfo.mods_count;
        for (unsigned int i = 0; i < n_boot_modules; i++) {
                MultibootModule *mods_arr = (MultibootModule *)_bootinfo.mods_addr;
                boot_modules[i]           = mods_arr[i];
                LogMessage(LOG_DEBUG,
                           "Boot module %d: Start address %p, End Address %p, Command Line %s", i,
                           boot_modules[i].start, boot_modules[i].end,
                           (char *)boot_modules[i].cmdline);
        }
}
Bool BootOptionProvided(const char *opt) {
        if (!(_bootinfo.flags & MULTIBOOT_CMDLINE)) return false;

        const char *iter      = boot_cmdline;
        Size        targetlen = strlen(opt);

        while (*iter != '\0') {
                while (*iter == ' ') iter++;
                if (*iter == '\0') break;

                const char *start = iter;
                while (*iter != ' ' && *iter != '\0') iter++;
                Size tokenlen = (Size)iter - (Size)start;

                if (tokenlen == targetlen && strcmp(opt, start) == 0) return true;
        }

        return false;
}
