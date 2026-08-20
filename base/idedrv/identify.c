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

#include "portdef.h"

#define ATA_STATUS_ERR     (0x01)
#define ATA_STATUS_IDX     (0x02)
#define ATA_STATUS_CORR    (0x04)
#define ATA_STATUS_DRQ     (0x08)
#define ATA_STATUS_DSC     (0x10)
#define ATA_STATUS_DF      (0x20)
#define ATA_STATUS_DRDY    (0x40)
#define ATA_STATUS_BSY     (0x80)

#define ATA_LBA28_MASK     (0x0FFFFFFF)
#define ATA_CMD_IDENTIFY   (0xEC)
#define ATA_CMD_READ       (0x20)

#define ATAPI_CMD_IDENTIFY (0xA1)
#define ATAPI_CMD_PACKET   (0xA0)

#define SECTOR_SIZE        (512)
#define CD_SECTOR_SIZE     (2048)
#define WORD_SIZE          (sizeof(u16))
#define ATA_READ_BUFSIZE   (SECTOR_SIZE / WORD_SIZE)

#define TO_LBA28(value)    ((value) &= (ATA_LBA28_MASK))

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

static void ATASoftResetDelay(u16 status_port) {
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
}

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

        outb(list.hddevsel, hddev);
        ATASoftResetDelay(list.status);

        /* Zero registers as required by spec */
        outb(list.seccount, 0);
        outb(list.lba0, 0);
        outb(list.lba1, 0);
        outb(list.lba2, 0);

        outb(list.command, ATA_CMD_IDENTIFY);
        ATASoftResetDelay(list.status);

        Byte status = inb(list.status);
        ATASoftResetDelay(list.status);
        if (!status) return STATUS_NOT_FOUND;

        /* Wait for BSY clear */
        while (status & ATA_STATUS_BSY) status = inb(list.status);

        /* Check if not ATA (probably ATAPI) */
        Byte lba1 = inb(list.lba1);
        Byte lba2 = inb(list.lba2);

        /* ATAPI drive. Not supported by this driver yet... */
        if (lba1 == 0x14 && lba2 == 0xEB) return STATUS_UNSUPPORTED;

        status = inb(list.status);
        while (!(status & ATA_STATUS_DRQ) && !(status & ATA_STATUS_ERR)) status = inb(list.status);
        if (status & ATA_STATUS_ERR) return STATUS_BAD;

        return STATUS_OK;
}
