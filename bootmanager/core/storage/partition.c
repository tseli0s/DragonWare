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

#include "cpu/bioscall.h"
#include "error.h"
#include "limits.h"
#include "mbr.h"
#include "storage/diskread.h"
#include "textmode/dbgprint.h"

static Partition system_partitions[MAX_SYSTEM_DRIVES][MAX_MBR_ENTRIES] = {0};

/* Welcome to the BIOS world. Everybody does their own shitfuckery I have to work around somehow.
 * After trying all sorts of random things, probing for INT 13H extensions was the most standardized
 * solution I could come up with. Can't wait to port DragonWare to UEFI. */
static Bool DriveIsPresent(int drive_num) {
        BIOSRegisters r = {
                .eax = 0x4100,
                .ebx = 0x55AA,
                .edx = drive_num & 0xFF /* Being a little paranoid here */
        };

        int cf = BIOSCall(0x13, &r);
        if (cf != 0) return false;

        /* We only care about the low 16 bits in this case, to avoid any high bit clobbers, we'll
         * mask them out */
        if ((r.ebx & 0xFFFF) != 0xAA55) return false;

        return true;
}

static Bool DriveIsCDROM(int drive_num) {
        /* Workaround for some weird BIOSes */
        if (drive_num >= 0xE0) return true;

        [[gnu::aligned(sizeof(u32))]]
        Byte packet[13] = {0};
        packet[0]       = sizeof(packet);

        BIOSRegisters r = {
                .eax = 0x4B01,
                .edx = (drive_num & 0xFF),
                .ds  = (u16)(((uintptr_t)packet >> 4) & 0xFFFF),
                .esi = (u16)((uintptr_t)packet & 0x000F),
        };
        return (BIOSCall(0x13, &r) == 0);
}

int InitPartitionTable(void) {
        DebugPrint("Probing available volume media, please wait...");

        int  disks_used       = 0;
        int  cds_used         = 0;
        int  hdds_used        = 0;
        int  drive_num        = 0x80;
        Byte bootsector[2048] = {0};
        ZeroMemory(system_partitions);

        /* The BIOS passes drive numbers during boot as a byte. Usually, floppies are between
         * 0x00-0x7F (If, for some reason, you've connected 128 floppies in your system), then
         * 0x80-0xDF are hard drives, and 0xE0 and beyond are CD-ROMs/DVDs.
         * To be sure we didn't miss anything, we will iterate through everything the BIOS has
         * detected. Note that in some older BIOSes, CDs may also appear as hard drives (so in the
         * range of 0x80-0xE0). Which is why there are additional checks below.
         * We start probing from the hard drives, as floppies are another story on their own (and
         * the checks below always succeed for some reason in QEMU).
         */
        for (drive_num = 0x80; drive_num <= 0xFF && disks_used < MAX_SYSTEM_DRIVES; drive_num++) {
                if (!DriveIsPresent(drive_num)) continue;
                if (DriveIsCDROM(drive_num)) {
                        /* Okay, briefly. This is a workaround for some quirk in my machine's CSM
                         * and VirtualBox. For whatever reason, it marks something (Definitely not
                         * what I'm booting off of) as a CD-ROM in the BIOS. So I was
                         * reading garbage from an unrelated device, and therefore couldn't locate
                         * the PVD to load the kernel in the ISO9660 driver. I have absolutely no
                         * idea why that happens. Adding this check makes it work again. If somebody
                         * is seeing this, I would love an explanation. I can't seem to find
                         * anything in the standard about it. If there's a better way than relying
                         * on heuristics, please let me know, this is such an ugly workaround. */
                        if (drive_num < 0xE0) {
                                continue;
                        }

                        Partition *curr   = &system_partitions[disks_used][0];
                        curr->drive_index = drive_num;
                        curr->lba_start   = 0;
                        curr->n_sectors   = U32_MAX; /* The volume spans the whole CD, so the entire
                                                        possible storage is assumed here. */
                        curr->present     = true;
                        curr->type        = PART_ISO9660_VOL;
                        snprintf(curr->identifier, sizeof(curr->identifier), "cd%u", cds_used);
                        disks_used++;
                        cds_used++;
                        continue;
                }
                if (ReadFromDisk(drive_num, 0, 1, bootsector) != STATUS_OK) {
                        DebugPrint(
                                "Failed to read boot sector from BIOS device number %d, "
                                "skipping parsing.",
                                drive_num);
                        continue;
                }
                MBRTable table;
                ReadMBRTable(bootsector, &table);

                for (int mbre = 0; mbre < MAX_MBR_ENTRIES; mbre++) {
                        if (!table[mbre].n_sectors_partition ||
                            table[mbre].partition_type == PART_EMPTY)
                                continue;

                        MBREntry   entry = table[mbre];
                        Partition *curr  = &system_partitions[disks_used][mbre];

                        snprintf(curr->identifier, IDENTIFIER_LIMIT, "hd%d/p%d", hdds_used, mbre);
                        curr->drive_index = drive_num;
                        curr->lba_start   = entry.lba_first_absolute_sector;
                        curr->n_sectors   = entry.n_sectors_partition;
                        curr->present     = true;
                        curr->type        = entry.partition_type;
                }
                hdds_used++;
                disks_used++;
        }
        if (!disks_used) {
                FatalError(
                        "No media has been detected by the bootloader. This is almost certainly a "
                        "bug. Please report this bug in the project's page. Some useful "
                        "information to supply when reporting this bug:\n"
                        "drive_num: 0x%x, disks_used: %d, cds_used: %d, hdds_used: %d",
                        drive_num, disks_used, cds_used, hdds_used);
        }
        return disks_used;
}

Partition GetPartitionEntryFromDisk(int index, int partition) {
        return system_partitions[index][partition];
}
