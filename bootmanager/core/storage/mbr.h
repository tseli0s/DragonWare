/**********************************************************************
 * FILE: mbr.c
 * PURPOSE: MBR partition scheme reading
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define MAX_MBR_ENTRIES     (4)
#define MBR_ENTRY_SIZE      (sizeof(MBREntry))
#define MBR_TABLE_SIZE      (MAX_MBR_ENTRIES * (MBR_ENTRY_SIZE))
#define MBR_BOOTSECT_OFFSET (446)

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _CHSAddress {
        Byte head, sector, cylinder; /* Reminder: bits 7-6 of the sector field contain the high
                                                   bits of the cylinder. */
} CHSAddress;

typedef struct [[gnu::packed]] _MBREntry {
        Byte       status; /* 0x80 = active, 0x0=inactive, others=unused */
        CHSAddress first_absolute_sector;
        Byte       partition_type;
        CHSAddress last_absolute_sector;
        u32        lba_first_absolute_sector;
        u32        n_sectors_partition;
} MBREntry;

typedef MBREntry MBRTable[4];

/* Returns 0 if the MBR table is present and proper, -1 otherwise. */
int ReadMBRTable(const Byte *restrict src, MBRTable *restrict dest);
