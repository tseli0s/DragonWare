/**********************************************************************
 * FILE: fs.c
 * PURPOSE: Lightweight filesystem abstraction primitives
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "fs.h"

#include <kstring.h>
#include <ktypes.h>
#include <mmutils.h>

#include "error.h"
#include "fat32/fat32.h"
#include "fs/iso9660/iso9660.h"
#include "storage/ata.h"
#include "storage/mbr.h"
#include "storage/partition.h"
#include "textmode/dbgprint.h"

#define SECTOR_SIZE 512

File OpenFile(const char *partition, const char *path) {
        Partition p     = {.lba_start = 0, .n_sectors = 0, .identifier = {0}, .type = 0};
        File      f     = {.internal = NullPointer,
                           .cursor   = 0,
                           .ended    = false,
                           .loaded   = false,
                           .t        = PART_EMPTY};
        bool      found = false;
        for (int i = 0; i < MAX_ATA_DISKS && !found; i++) {
                for (int j = 0; j < MAX_MBR_ENTRIES && !found; j++) {
                        p = GetPartitionEntryFromDisk(i, j);
                        if (p.n_sectors != 0 && p.type != PART_EMPTY) {
                                if (strcmp(p.identifier, partition) == 0) found = true;
                        }
                }
        }

        if (!found) {
                f.loaded = false;
                return f;
        } else {
                DebugPrint("Found matching partition: Start LBA %u, length (LBA) %u, type: 0x%x",
                           p.lba_start, p.n_sectors, p.type);
        }

        switch (p.type) {
                case PART_FAT32_CHS:
                case PART_FAT32_LBA: {
                        f.t = PART_FAT32_LBA;
                        if (OpenFile_FAT32(p, path, &f) != STATUS_OK) {
                                f.loaded = false;
                                return f;
                        }
                        break;
                }
                case PART_ISO9660_VOL:
                case PART_ISO9660_VOL2: {
                        f.t = PART_ISO9660_VOL;
                        if (OpenFile_ISO9660(p, path, &f) != STATUS_OK) {
                                f.loaded = false;
                                return f;
                        }
                        break;
                }
                default:
                        FatalError("Unknown partition type");
                        return f;
        }

        return f;
}

/* Reads n_bytes from file and places them in output */
Size ReadFromFile(File *f, Byte *output, Size n_bytes) {
        switch (f->t) {
                case PART_FAT32_CHS:
                case PART_FAT32_LBA: {
                        int result = ReadFile_FAT32(f, n_bytes, output);
                        if ((Size)result != n_bytes)
                                FatalError(
                                        "Cannot read file contents from FAT32 volume, expected %d "
                                        "bytes but read %d",
                                        n_bytes, result);
                        else
                                return n_bytes;

                        break;
                }
                case PART_ISO9660_VOL:
                case PART_ISO9660_VOL2: {
                        int result = ReadFile_ISO9660(f, n_bytes, output);
                        if ((Size)result != n_bytes)
                                FatalError(
                                        "Cannot read file contents from ISO9660 volume, expected "
                                        "%d bytes but read %d",
                                        n_bytes, result);
                        else
                                return n_bytes;

                        break;
                }
                case PART_EMPTY:
                default:
                        return 0;
        }
        return n_bytes;
}

void CloseFile(File *f) {
        f->cursor   = 0;
        f->ended    = true;
        f->internal = NullPointer;
        f->loaded   = false;
}
