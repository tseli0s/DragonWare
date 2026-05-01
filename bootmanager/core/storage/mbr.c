/**********************************************************************
 * FILE: mbr.c
 * PURPOSE: MBR partition scheme reading
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "mbr.h"

#include <mmutils.h>

int ReadMBRTable(const Byte *restrict src, MBRTable *restrict dest) {
        if (!src || !dest) return -1;
        if (src[510] != 0x55 || src[511] != 0xAA) return -1;

        kzeromem(dest, MBR_TABLE_SIZE);
        for (int i = 0; i < MAX_MBR_ENTRIES; i++) {
                Size     off = (i * sizeof(MBREntry)) + MBR_BOOTSECT_OFFSET;
                MBREntry e;
                memcpy(&e, src + off, MBR_ENTRY_SIZE);

                /* Assume not a valid MBR entry */
                if (e.n_sectors_partition == 0 || e.partition_type == 0) {
                        kzeromem(&(*dest)[i], MBR_ENTRY_SIZE);
                        continue;
                } else
                        memcpy(&(*dest)[i], src + off, MBR_ENTRY_SIZE);
        }
        return 0;
}
