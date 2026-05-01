/**********************************************************************
 * FILE: partition.h
 * PURPOSE: Partition management
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* hd0p0, hd1p1, ... 12 bytes will suffice */
#define IDENTIFIER_LIMIT (12)

typedef enum _PartitionType : unsigned char {
        PART_EMPTY = 0x00,

        /* FAT */
        PART_FAT12         = 0x01,
        PART_FAT16_CHS     = 0x04,
        PART_EXTENDED_CHS  = 0x05,
        PART_FAT16_LBA     = 0x06,
        PART_FAT32_CHS     = 0x0B,
        PART_FAT32_LBA     = 0x0C,
        PART_FAT16_LBA_ALT = 0x0E,
        PART_EXTENDED_LBA  = 0x0F,

        /* CD-ROM, this is only used internally and almost never present in real disks */
        PART_ISO9660_VOL  = 0xCD,
        PART_ISO9660_VOL2 = 0xDB,

        /* Protective MBR for GPT */
        PART_GPT_PROTECTIVE = 0xEE,
        PART_EFI_SYSTEM     = 0xEF,
} PartitionType;

typedef struct _Partition {
        u32  drive_index; /* To which drive does this partition belong to */
        u32  lba_start;
        u32  n_sectors;
        char identifier[IDENTIFIER_LIMIT]; /* Set by the bootloader */
        Byte type;
        Bool present;
} Partition;

/* Returns the amount of disks in the system that have a valid partition table */
int InitPartitionTable(void);

/* Returns the partition entry at the given disk */
Partition GetPartitionEntryFromDisk(int index, int partition);
