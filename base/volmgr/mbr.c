/**********************************************************************
 * FILE: mbr.c
 * PURPOSE: MBR partition scheme support
 * PROJECT: DragonWare Base System
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "mbr.h"

#include <kerneltypes.h>
#include <string.h>

#define MBR_SIGNATURE ((u16)0xAA55)

Status ReadMBRTable(const Byte src[static 512], MBREntry dest[static 4]) {
        static const u16 mbr_sig = MBR_SIGNATURE;
        if (memcmp(src + MBR_BOOTSECT_OFFSET + MBR_TABLE_SIZE, &mbr_sig, sizeof(u16)) != 0)
                return STATUS_NOT_FOUND;

        int n = 0;
        for (int i = 0; i < MAX_MBR_ENTRIES; i++) {
                Size     off = (Size)((i * sizeof(MBREntry)) + MBR_BOOTSECT_OFFSET);
                MBREntry e;
                memcpy(&e, src + off, MBR_ENTRY_SIZE);

                if (e.n_sectors_partition == 0 || e.partition_type == 0) {
                        goto invalid;
                } else {
                        memcpy(&dest[i], src + off, MBR_ENTRY_SIZE);
                        n++;
                        continue;
                }
        invalid:
                memset(&dest[i], 0, MBR_ENTRY_SIZE);
        }

        return (n > 0) ? STATUS_OK : STATUS_NOT_FOUND;
}
