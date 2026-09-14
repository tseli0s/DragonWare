/**********************************************************************
 * FILE: mbr.h
 * PURPOSE: MBR partition scheme support
 * PROJECT: DragonWare Base System
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define MAX_MBR_ENTRIES     (4)
#define MBR_ENTRY_SIZE      (sizeof(MBREntry))
#define MBR_TABLE_SIZE      (MAX_MBR_ENTRIES * (MBR_ENTRY_SIZE))
#define MBR_BOOTSECT_OFFSET (446)

#include <kerneltypes.h>

/* Definitely didn't copypaste all this from bootmanager/storage/mbr.h :P */
typedef struct [[gnu::packed]] _CHSAddress {
        Byte head, sector, cylinder; /* Reminder: bits 7-6 of the sector field contain the high
                                                   bits of the cylinder. */
} CHSAddress;

/**
 * @brief A single partition entry in an MBR-formatted storage medium.
 * @sa ReadMBRTable
 */
typedef struct [[gnu::packed]] _MBREntry {
        Byte       status; /* 0x80 = active, 0x0=inactive, others=unused */
        CHSAddress first_absolute_sector;
        Byte       partition_type;
        CHSAddress last_absolute_sector;
        u32        lba_first_absolute_sector;
        u32        n_sectors_partition;
} MBREntry;

typedef MBREntry MBRTable[4];

/**
 * @brief Parses an MBR partition table and returns @ref MBREntry values into @p dest
 * @param[in] src The first 512 byte sector of the drive to parse the MBR table from.
 * @param[out] dest Pointer to store the partition entries to.
 * @note If an entry is considered invalid (eg. 0-length partition) dest[n] will be zeroed out,
 * where n is the entry being currently parsed.
 * @returns STATUS_OK on success, STATUS_NOT_FOUND if the drive doesn't have an MBR table.
 */
Status ReadMBRTable(const Byte src[static 512], MBREntry dest[static 4]);
