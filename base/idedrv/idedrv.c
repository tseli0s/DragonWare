/**********************************************************************
 * FILE: idedrv.c
 * PURPOSE: IDE/ATA userspace disk driver entry point
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "identify.h"
#include "portdef.h"

static inline void die(const char *msg) {
        printf("fatal error: %s\n", msg);
        exit(EXIT_FAILURE);
}

int main(void) {
        const u16 ata_ports[] = {
                ATA_CTRL_PRIMARY,       ATA_CTRL_SECONDARY,     ATA_PRIMARY_BASE,
                ATA_SECONDARY_BASE,     ATA_PRIMARY_BASE + 1,   ATA_PRIMARY_BASE + 2,
                ATA_PRIMARY_BASE + 3,   ATA_PRIMARY_BASE + 4,   ATA_PRIMARY_BASE + 5,
                ATA_PRIMARY_BASE + 6,   ATA_PRIMARY_BASE + 7,   ATA_SECONDARY_BASE + 1,
                ATA_SECONDARY_BASE + 2, ATA_SECONDARY_BASE + 3, ATA_SECONDARY_BASE + 4,
                ATA_SECONDARY_BASE + 5, ATA_SECONDARY_BASE + 6, ATA_SECONDARY_BASE + 7,
        };
        if (_DWRequestPorts(ata_ports, sizeof(ata_ports) / sizeof(ata_ports[0])) != STATUS_OK)
                die("Cannot access ATA/IDE I/O ports, permission denied from the kernel.");
        
        for (int bus = 0; bus <= 1; bus++) {
                for (int master = 0; master <= 1; master++) {
                        if (IdentifyDrive(bus, master) != STATUS_OK)
                                continue;
                        else printf("Detected connected ATA/IDE compatible medium on bus %d drive %d\n");
                }
        }
        return 0;
}
