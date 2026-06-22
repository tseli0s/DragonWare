/**********************************************************************
 * FILE: diskread.c
 * PURPOSE: BIOS-based disk access implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "diskread.h"

#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "common/frame.h"
#include "cpu/bioscall.h"

/* Used for INT 13H with the EDD extensions. If we're on this stage, the extensions are supported,
 * as it is the only way to load stage 2 at the moment.
 */
typedef struct [[gnu::packed]] _BIOSDiskAccessPacket {
        Byte size;
        Byte __zero;
        u16  n_sectors;
        u16  offset;  /* seg:OFFSET */
        u16  segment; /* SEG:offset */
        u64  lba;
} BIOSDiskAccessPacket;

/*
 * Used to get the sector size for a drive (see GetSectorSizeForDrive())
 * Part of the EDD specification: http://www.o3one.org/hwdocs/bios_doc/bios_specs_edd30.pdf
 */
typedef struct [[gnu::packed]] _BIOSExtendedDriveParameters {
        u16 size;
        u16 flags;
        u32 cylinders;
        u32 heads;
        u32 sectors_per_track;
        u64 sectors_total;
        u16 bytes_per_sector;
        u32 edd_config;
} BIOSExtendedDriveParameters;

/* This only works in low memory pointers */
static Status __read_from_disk_lomem(int drive, u64 lba, int n_sectors, void *dest) {
        /* The standard requires 2 byte alignment:
         * https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)#LBA_in_Extended_Mode
         */
        [[gnu::aligned(2)]]
        BIOSDiskAccessPacket dap = {.size      = sizeof(BIOSDiskAccessPacket),
                                    .__zero    = 0,
                                    .n_sectors = n_sectors,
                                    .offset    = (u16)((uintptr_t)dest & 0x000F),
                                    .segment   = (u16)(((uintptr_t)dest >> 4) & 0xFFFF),
                                    .lba       = lba};

        BIOSRegisters r = {
                .eax = 0x4200,
                .edx = drive & 0xFF,
                .ds  = (u16)(((uintptr_t)&dap >> 4) & 0xFFFF),
                .esi = (u16)((uintptr_t)&dap & 0x000F),
        };
        int status = BIOSCall(0x13, &r);
        return (status == 0) ? STATUS_OK : STATUS_BAD;
}

static int GetSectorSizeForDrive(u8 drive) {
        /*
         * First, try to ask the BIOS for information. That is the most accurate and safe option,
         * but may not be supported on all hardware.
         * (https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=48h:_Extended_Read_Drive_Parameters)
         */
        BIOSExtendedDriveParameters p      = {0};
        BIOSRegisters               r      = {.eax = 0x4800,
                                              .edx = drive,
                                              .ds  = (u16)(((uintptr_t)&p >> 4) & 0xFFFF),
                                              .esi = (u16)((uintptr_t)&p & 0x000F)};
        int                         status = BIOSCall(0x13, &r);
        if (status == 0)
                return p.bytes_per_sector;
        else {
                /* The BIOS failed. Now we rely on the drive number. Usually, 0xE0 and beyond are
                 * CD-ROMs with a sector size of 2048 bytes, and hard disks have a sector size of
                 * 512 bytes. This is dangerous, but better than nothing. */
                return (drive >= 0xE0) ? 2048 : SECTOR_SIZE;
        }
}

Status ReadFromDisk(int drive, u64 lba, int n_sectors, void *dest) {
        uintptr_t   addr = (uintptr_t)dest;
        static Byte tmpbuf[FRAME_SIZE];

        int sector_size           = GetSectorSizeForDrive(drive);
        int max_sectors_per_chunk = FRAME_SIZE / sector_size;

        if (addr >= 0x100000) {
                while (n_sectors > 0) {
                        int chunk = (n_sectors > max_sectors_per_chunk) ? max_sectors_per_chunk
                                                                        : n_sectors;

                        if (__read_from_disk_lomem(drive, lba, chunk, tmpbuf) != STATUS_OK)
                                return STATUS_BAD;

                        memcpy((void *)addr, tmpbuf, chunk * sector_size);

                        lba += chunk;
                        addr += chunk * sector_size;
                        n_sectors -= chunk;
                }
                return STATUS_OK;
        } else
                return __read_from_disk_lomem(drive, lba, n_sectors, dest);
}