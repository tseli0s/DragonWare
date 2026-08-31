/**********************************************************************
 * FILE: identify.c
 * PURPOSE: IDE/ATA drive IDENTIFY command helpers
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "identify.h"

#include <io.h>
#include <kerneltypes.h>

#include "idedrv/ctrl.h"
#include "portdef.h"

typedef struct _ATAPortList {
        u16 data;
        u16 error;
        u16 seccount;
        u16 lba0;
        u16 lba1;
        u16 lba2;
        u16 hddevsel;
        u16 status;
        u16 command;
        u16 alt_status;
} ATAPortList;

static void ATASelectDevicePorts(int bus, ATAPortList *list) {
        if (bus == 0) {
                list->data     = ATA_DATA_PRIMARY;
                list->seccount = ATA_SECCOUNT_PRIMARY;
                list->lba0     = ATA_LBA0_PRIMARY;
                list->lba1     = ATA_LBA1_PRIMARY;
                list->lba2     = ATA_LBA2_PRIMARY;
                list->hddevsel = ATA_HDDEVSEL_PRIMARY;
                list->status   = ATA_STATUS_PRIMARY;
                list->command  = ATA_COMMAND_PRIMARY;
        } else {
                list->data     = ATA_DATA_SECONDARY;
                list->seccount = ATA_SECCOUNT_SECONDARY;
                list->lba0     = ATA_LBA0_SECONDARY;
                list->lba1     = ATA_LBA1_SECONDARY;
                list->lba2     = ATA_LBA2_SECONDARY;
                list->hddevsel = ATA_HDDEVSEL_SECONDARY;
                list->status   = ATA_STATUS_SECONDARY;
                list->command  = ATA_COMMAND_SECONDARY;
        }
}

Status IdentifyDrive(int primary, int master) {
        ATAPortList list;
        ATASelectDevicePorts(primary, &list);

        Byte hddev = master ? 0xB0 : 0xA0;

        /* Check if the bus is floating. Now I'm typing this off of memory so my explanation may be
         * wrong, but basically, if nothing is connected in that port, all the lines are pulled high
         * upon the read and nothing pulls them back down, so all bits are set to 1, resulting in
         * 0xFF returned when reading from that port (because all bits are set). */
        Byte floating = inb(list.status);
        if (floating == 0xFF) return STATUS_NOT_FOUND;

        while (inb(list.status) & ATA_STATUS_BSY);

        /* There's a bug (due to how IDE and the microkernel works): If interrupts are enabled, an
         * interrupt will arrive at IRQ14, except we didn't create a listener port at this point, so
         * there's nothing to catch it and pull the INTRQ line down again (or whatever needs to
         * happen, sorry, not an electrical engineer). And no more interrupts will arrive.
         */
        DisableINTRQ(primary);
        outb(list.hddevsel, hddev);
        Wait400ns(primary);

        /* Zero registers as required by spec */
        outb(list.seccount, 0);
        outb(list.lba0, 0);
        outb(list.lba1, 0);
        outb(list.lba2, 0);

        outb(list.command, ATA_CMD_IDENTIFY);
        Wait400ns(primary);

        Byte status = inb(list.status);
        Wait400ns(primary);
        if (!status) return STATUS_NOT_FOUND;

        /* Wait for BSY clear */
        while (status & ATA_STATUS_BSY) status = inb(list.status);

        /* Check if not ATA (probably ATAPI) */
        Byte lba1 = inb(list.lba1);
        Byte lba2 = inb(list.lba2);
        if (lba1 == 0x14 && lba2 == 0xEB) {
                /* ATAPI drive, unsupported */
                return STATUS_UNSUPPORTED;
        }

        status = inb(list.status);
        while (!(status & ATA_STATUS_DRQ) && !(status & ATA_STATUS_ERROR))
                status = inb(list.status);
        if (status & ATA_STATUS_ERROR) return STATUS_BAD;

        /* Apparently, we need to drain the IDENTIFY data as well, otherwise the drive won't
         * raise more interrupts. Seems to be emulator dependent (QEMU won't issue subsequent
         * interrupts, Bochs doesn't care). Nonetheless, we need to drain the sink.
         */
        for (Size i = 0; i < 256; i++) (void)inw(list.data);
        return STATUS_OK;
}
