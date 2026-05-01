/**********************************************************************
 * FILE: partition.c
 * PURPOSE: Partition management
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "partition.h"

#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "ata.h"
#include "mbr.h"
#include "textmode/dbgprint.h"

static Partition system_partitions[MAX_ATA_DISKS][MAX_MBR_ENTRIES] = {0};

static Bool DriveIsPresent(int index) { return ATADriveIsPresent(index); }

int InitPartitionTable(void) {
        int  disks_used      = 0;
        Byte bootsector[512] = {0};
        ZeroMemory(system_partitions);
        for (int disk = 0; disk < MAX_ATA_DISKS; disk++) {
                if (!DriveIsPresent(disk)) continue;
                if (ATADriveIsATAPI(disk)) {
                        /* Assume this is a CD-ROM, so fake a partition, that spans the whole volume
                         */
                        Partition *curr   = &system_partitions[disk][0];
                        curr->drive_index = disk;
                        curr->lba_start   = 0;
                        curr->n_sectors   = 2048; /* FIXME */
                        curr->present     = true;
                        curr->type        = PART_ISO9660_VOL;
                        strncpy(curr->identifier, "cd0", IDENTIFIER_LIMIT);
                        curr->identifier[IDENTIFIER_LIMIT - 1] = '\0';
                        disks_used++;
                        continue;
                }

                MBRTable table;
                if (!ATAReadSectors(disk, 1, 0, bootsector)) {
                        DebugPrint("Unable to read from device index %d, skipping parsing.", disk);
                        continue;
                }
                ReadMBRTable(bootsector, &table);

                for (int mbre = 0; mbre < MAX_MBR_ENTRIES; mbre++) {
                        if (!table[mbre].n_sectors_partition ||
                            table[mbre].partition_type == PART_EMPTY)
                                continue;

                        MBREntry   entry = table[mbre];
                        Partition *curr  = &system_partitions[disk][mbre];

                        snprintf(curr->identifier, IDENTIFIER_LIMIT, "hd%d/p%d", disk, mbre);
                        curr->identifier[IDENTIFIER_LIMIT - 1] = '\0';
                        curr->drive_index                      = disk;
                        curr->lba_start                        = entry.lba_first_absolute_sector;
                        curr->n_sectors                        = entry.n_sectors_partition;
                        curr->present                          = true;
                        curr->type                             = entry.partition_type;
                }
                disks_used++;
        }
        return disks_used;
}

Partition GetPartitionEntryFromDisk(int index, int partition) {
        return system_partitions[index][partition];
}
