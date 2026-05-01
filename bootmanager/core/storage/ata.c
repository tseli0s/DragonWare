/**********************************************************************
 * FILE: ata.c
 * PURPOSE: ATA/IDE support code implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "ata.h"

#include <iodelay.h>
#include <ioport.h>
#include <ktypes.h>
#include <mmutils.h>

#include "error.h"
#include "textmode/dbgprint.h"

#define ATA_PRIMARY_BASE       (0x1F0)
#define ATA_SECONDARY_BASE     (0x170)
#define ATA_CTRL_PRIMARY       (0x3F6)
#define ATA_CTRL_SECONDARY     (0x376)

#define ATA_DATA_PRIMARY       (ATA_PRIMARY_BASE + 0)
#define ATA_ERROR_PRIMARY      (ATA_PRIMARY_BASE + 1)
#define ATA_SECCOUNT_PRIMARY   (ATA_PRIMARY_BASE + 2)
#define ATA_LBA0_PRIMARY       (ATA_PRIMARY_BASE + 3)
#define ATA_LBA1_PRIMARY       (ATA_PRIMARY_BASE + 4)
#define ATA_LBA2_PRIMARY       (ATA_PRIMARY_BASE + 5)
#define ATA_HDDEVSEL_PRIMARY   (ATA_PRIMARY_BASE + 6)
#define ATA_STATUS_PRIMARY     (ATA_PRIMARY_BASE + 7)
#define ATA_COMMAND_PRIMARY    (ATA_PRIMARY_BASE + 7)

#define ATA_DATA_SECONDARY     (ATA_SECONDARY_BASE + 0)
#define ATA_ERROR_SECONDARY    (ATA_SECONDARY_BASE + 1)
#define ATA_SECCOUNT_SECONDARY (ATA_SECONDARY_BASE + 2)
#define ATA_LBA0_SECONDARY     (ATA_SECONDARY_BASE + 3)
#define ATA_LBA1_SECONDARY     (ATA_SECONDARY_BASE + 4)
#define ATA_LBA2_SECONDARY     (ATA_SECONDARY_BASE + 5)
#define ATA_HDDEVSEL_SECONDARY (ATA_SECONDARY_BASE + 6)
#define ATA_STATUS_SECONDARY   (ATA_SECONDARY_BASE + 7)
#define ATA_COMMAND_SECONDARY  (ATA_SECONDARY_BASE + 7)

#define ATA_STATUS_ERR         (0x01)
#define ATA_STATUS_IDX         (0x02)
#define ATA_STATUS_CORR        (0x04)
#define ATA_STATUS_DRQ         (0x08)
#define ATA_STATUS_DSC         (0x10)
#define ATA_STATUS_DF          (0x20)
#define ATA_STATUS_DRDY        (0x40)
#define ATA_STATUS_BSY         (0x80)

#define ATA_LBA28_MASK         (0x0FFFFFFF)
#define ATA_CMD_IDENTIFY       (0xEC)
#define ATA_CMD_READ           (0x20)

#define ATAPI_CMD_IDENTIFY     (0xA1)
#define ATAPI_CMD_PACKET       (0xA0)

#define SECTOR_SIZE            (512)
#define CD_SECTOR_SIZE         (2048)
#define WORD_SIZE              (sizeof(u16))
#define ATA_READ_BUFSIZE       (SECTOR_SIZE / WORD_SIZE)

#define TO_LBA28(value)        ((value) &= (ATA_LBA28_MASK))

typedef struct _ATADeviceSlot {
        int  bus;
        int  master; /* 1 = master, 0 = slave */
        Bool present;
        Bool atapi; /* ATAPI drives need separate commands (SCSI something idk I haven't read on it
                       yet)*/
        u16  buffer[256];
} ATADeviceSlot;

static ATADeviceSlot devices[MAX_ATA_DISKS] = {0};

static void ATASwapStringBytes(char *s, int len) {
        for (int i = 0; i < len; i += 2) {
                char t   = s[i];
                s[i]     = s[i + 1];
                s[i + 1] = t;
        }
}

static void ATASoftResetDelay(u16 status_port) {
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
        inb(status_port);
}

static void ATASelectDevicePorts(int bus, u16 *data, u16 *seccount, u16 *lba0, u16 *lba1, u16 *lba2,
                                 u16 *hddevsel, u16 *status, u16 *command) {
        if (bus == 0) {
                *data     = ATA_DATA_PRIMARY;
                *seccount = ATA_SECCOUNT_PRIMARY;
                *lba0     = ATA_LBA0_PRIMARY;
                *lba1     = ATA_LBA1_PRIMARY;
                *lba2     = ATA_LBA2_PRIMARY;
                *hddevsel = ATA_HDDEVSEL_PRIMARY;
                *status   = ATA_STATUS_PRIMARY;
                *command  = ATA_COMMAND_PRIMARY;
        } else {
                *data     = ATA_DATA_SECONDARY;
                *seccount = ATA_SECCOUNT_SECONDARY;
                *lba0     = ATA_LBA0_SECONDARY;
                *lba1     = ATA_LBA1_SECONDARY;
                *lba2     = ATA_LBA2_SECONDARY;
                *hddevsel = ATA_HDDEVSEL_SECONDARY;
                *status   = ATA_STATUS_SECONDARY;
                *command  = ATA_COMMAND_SECONDARY;
        }
}

static int ATAIdentify(u16 *buffer, int bus, int master) {
        ATAError drive_status = ATA_ERR_NONE;
        u16 data_port, sec_count_port, lba0_port, lba1_port, lba2_port, hddevsel_port, status_port,
                command_port;

        ATASelectDevicePorts(bus, &data_port, &sec_count_port, &lba0_port, &lba1_port, &lba2_port,
                             &hddevsel_port, &status_port, &command_port);
        Byte hddev = master ? 0xA0 : 0xB0;
        while (inb(status_port) & ATA_STATUS_BSY);

        outb(hddevsel_port, hddev);
        ATASoftResetDelay(status_port);

        /* Zero registers as required by spec */
        outb(sec_count_port, 0);
        outb(lba0_port, 0);
        outb(lba1_port, 0);
        outb(lba2_port, 0);

        outb(command_port, ATA_CMD_IDENTIFY);
        ATASoftResetDelay(status_port);

        Byte status = inb(status_port);
        ATASoftResetDelay(status_port);
        if (!status) return ATA_ERR_NODEV;

        /* Wait for BSY clear */
        while (status & ATA_STATUS_BSY) status = inb(status_port);

        /* Check if not ATA (probably ATAPI) */
        Byte lba1 = inb(lba1_port);
        Byte lba2 = inb(lba2_port);

        if (lba1 == 0x14 && lba2 == 0xEB) {
                drive_status = ATA_ERR_ATAPI;
                /* Need to reselect, there are some buggy drives out there */
                outb(hddevsel_port, hddev);
                ATASoftResetDelay(status_port);

                /* This is an ATAPI device, we must send a different command. */
                outb(command_port, ATAPI_CMD_IDENTIFY);
                ATASoftResetDelay(status_port);

                Byte atapi_status = inb(status_port);
                ATASoftResetDelay(status_port);
                if (!atapi_status) return ATA_ERR_NODEV;

                /* Wait for BSY clear */
                while (inb(status_port) & ATA_STATUS_BSY);
        }

        status = inb(status_port);
        while (!(status & ATA_STATUS_DRQ) && !(status & ATA_STATUS_ERR)) status = inb(status_port);
        if (status & ATA_STATUS_ERR) return ATA_DEVERR;

        for (unsigned int i = 0; i < SECTOR_SIZE / (sizeof(u16)); i++) buffer[i] = inw(data_port);

        /* Swap strings for human-readable serial, firmware, model. The devices have them backwards,
         * not sure why*/
        ATAIdentifyData *data = (ATAIdentifyData *)buffer;
        ATASwapStringBytes(data->serial, sizeof(data->serial));
        ATASwapStringBytes(data->firmware, sizeof(data->firmware));
        ATASwapStringBytes(data->model, sizeof(data->model));

        return drive_status;
}

static inline int ATAIdentifySlot(int bus, int master, u16 *buffer) {
        return ATAIdentify(buffer, bus, master);
}

static inline Status WaitForDRQ(u16 status_port) {
        u64 timeout = 0;
        while (!(inb(status_port) & ATA_STATUS_DRQ)) {
                timeout++;
                IODelay();
                if (timeout > 20000000) {
                        DebugPrint("ATA: %s reached timeout before DRQ set!", __PRETTY_FUNCTION__);
                        return STATUS_TIMEOUT;
                }
        }
        return STATUS_OK;
}

Bool ATADriveIsPresent(int index) { return devices[index].present; }

int ATAIdentifyAllDevices(int max_devices) {
        int found = 0;

        for (int bus = 0; bus <= 1; bus++) {
                for (int master = 1; master >= 0; master--) {
                        /* Formula I devised (or whatever the word is). It works because
                         * multiplication by zero always yields zero, thanks maths :)
                         * If you replace bus and master for all possible values, you will see it
                         * makes sense. But as an informative comment:
                         * bus = 0, master = 1 -> 0*2 + 1-1 = index 0 (Indeed, primary master)
                         * bus = 0, master = 0 -> 0*2 + 1-0 = index 1 (Yep, primary slave)
                         * bus = 1, master = 0 -> 1*2 + 1-0 = index 2 (See? Secondary master)
                         * bus = 1, master = 1 -> 1*2 + 1-1 = index 3 (>>>            slave)
                         * See the documentation in ata.h as well for even more explanation
                         */
                        int index = (bus * 2) + (1 - master);

                        if (index >= max_devices) continue;

                        ATADeviceSlot *slot = &devices[index];
                        slot->bus           = bus;
                        slot->master        = master;

                        int ret = ATAIdentifySlot(bus, master, slot->buffer);
                        if (ret == 0) {
                                slot->present = true;
                                slot->atapi   = false;
                        } else if (ret == ATA_ERR_ATAPI) {
                                slot->present = true;
                                slot->atapi   = true;
                        } else
                                slot->present = false;

                        if (slot->present) {
                                ATAIdentifyData *read_data = (ATAIdentifyData *)slot->buffer;
                                DebugPrint(
                                        "ATA Drive %d: Serial %s, Model %s, Firmware %s, "
                                        "%d sectors",
                                        index, read_data->serial, read_data->model,
                                        read_data->firmware, (u32)read_data->lba28_sectors);
                                found++;
                        }
                }
        }
        return found;
}
/*
 * ATAPI-based drives need a separate reading method.
 * Even though most CDs use 2048 byte sectors, we will "fake" 512 byte multiples and simply discard
 * the extra data as needed. This will make the bootloader work on more machines, and I've already
 * started planning for more platform support, so I'm fine with this "tradeoff" and I can fix it
 * later.
 */
static Size ATAPIReadSectors(int index, Size n, u32 lbastart, Byte *dest) {
        u16 dataport, seccount_port, lba0_port, lba1_port, lba2_port, hddevsel_port, status_port,
                command_port;
        /* yk, just in case */
        if (unlikely(!ATADriveIsATAPI(index) || !ATADriveIsPresent(index))) return 0;

        int bus    = devices[index].bus;
        int master = devices[index].master;

        ATASelectDevicePorts(bus, &dataport, &seccount_port, &lba0_port, &lba1_port, &lba2_port,
                             &hddevsel_port, &status_port, &command_port);

        /* Here we don't want the LBA bit set (unlike ATAReadSectors below), which is why we use
         * 0xA0/B0 instead of 0xE0/F0.
         * (Bit description: 7 = Always set, 6 = LBA (<-- NOT SETTING THAT), 5 = Always set, 4 = 0
         * for master/1 for slave, 3-0 = Address bits for ATA PIO, all cleared in ATAPI)
         */
        Byte hddev = master ? 0xA0 : 0xB0;
        outb(hddevsel_port, hddev);
        ATASoftResetDelay(status_port);
        while (inb(status_port) & (ATA_STATUS_BSY | ATA_STATUS_DRQ));

        /* That's 0x800 = 2048 in hex, but little endian order so the MSB goes last */
        outb(lba1_port, 0x00);
        outb(lba2_port, 0x08);

        /* Convert ATAPI LBA to plain hard disk LBA. Because ATAPI uses 2048 byte sectors, but hard
         * drives use 512 bytes sectors, and the rest of the bootloader expects 512 bytes per read
         */
        u32 atapi_lba    = lbastart / 4;
        u32 offset       = lbastart % 4;           /* in 512-byte sectors */
        u32 total_needed = offset + n;             /* 512 byte sectors needed */
        u32 atapi_count  = (total_needed + 3) / 4; /* Alignment */

        /* Send ATAPI packet command. TODO: Stop using magic numbers here. */
        outb(command_port, ATAPI_CMD_PACKET);
        ATASoftResetDelay(status_port);

        while (inb(status_port) & ATA_STATUS_BSY);
        if (WaitForDRQ(status_port) != STATUS_OK) return 0;

        [[gnu::aligned(sizeof(u16))]]
        u8 scsi_packet[12] = {0};
        scsi_packet[0]     = 0x28; /* READ command */
        scsi_packet[2]     = (atapi_lba >> 24) & 0xFF;
        scsi_packet[3]     = (atapi_lba >> 16) & 0xFF;
        scsi_packet[4]     = (atapi_lba >> 8) & 0xFF;
        scsi_packet[5]     = atapi_lba & 0xFF;
        scsi_packet[7]     = (atapi_count >> 8) & 0xFF;
        scsi_packet[8]     = atapi_count & 0xFF;
        scsi_packet[9]     = 0x0;

        u16 *packetptr = (u16 *)scsi_packet;
        for (unsigned int i = 0; i < sizeof(scsi_packet) / sizeof(u16); i++)
                outw(dataport, packetptr[i]);

        /* Temporary buffer. We read here and copy whatever we need into dest. */
        u16  tmpbuf[CD_SECTOR_SIZE / sizeof(u16)] = {0};
        Size sectors_read                         = 0;

        for (Size a = 0; a < atapi_count; a++) {
                while (inb(status_port) & ATA_STATUS_BSY);
                if (WaitForDRQ(status_port) != STATUS_OK) return sectors_read;
                for (Size j = 0; j < CD_SECTOR_SIZE / sizeof(u16); j++) tmpbuf[j] = inw(dataport);

                u32 start = (a == 0) ? offset : 0;
                u32 end   = 4;

                if (a == atapi_count - 1) {
                        u32 remaining = n - sectors_read;
                        if (remaining < 4) end = start + remaining;
                }

                for (u32 s = start; s < end; s++) {
                        /* copy one 512-byte sector */
                        memcpy(dest + sectors_read * SECTOR_SIZE,
                               ((Byte *)tmpbuf) + s * SECTOR_SIZE, SECTOR_SIZE);
                        sectors_read++;
                }
        }
        return sectors_read;
}

Size ATAReadSectors(int index, Size n, u32 lbastart, Byte *dest) {
        if (index > 3 || index < 0) return 0;
        if (!n || !dest) return 0;
        if (!ATADriveIsPresent(index)) return 0;

        /* Truncate n if the requested sectors are too many, as we don't support it yet. */
        if (n > MAX_SECTORS_PER_READ) {
                DebugPrint(
                        "Attempted to read >MAX_SECTORS_PER_READ (%d) sectors. Truncating to %d.",
                        n, MAX_SECTORS_PER_READ);
                n = MAX_SECTORS_PER_READ;
        }

        /* Avoid any 28 bit overflows to be sure */
        TO_LBA28(lbastart);

        if (devices[index].atapi) return ATAPIReadSectors(index, n, lbastart, dest);

        u16 dataport, seccount_port, lba0_port, lba1_port, lba2_port, hddevsel_port, status_port,
                command_port;
        int bus    = devices[index].bus;
        int master = devices[index].master;

        ATASelectDevicePorts(bus, &dataport, &seccount_port, &lba0_port, &lba1_port, &lba2_port,
                             &hddevsel_port, &status_port, &command_port);

        /* Select the device we want to use */
        Byte hddev = (master ? 0xE0 : 0xF0) | ((lbastart >> 24) & 0x0F);
        outb(hddevsel_port, hddev);
        ATASoftResetDelay(status_port);

        /* We have to wait in case the drive is busy with something else */
        while (inb(status_port) & ATA_STATUS_BSY);

        /* Send the starting sector to the controller. We need to send each byte manually */
        outb(seccount_port, n);
        outb(lba0_port, (Byte)(lbastart & 0xff));
        outb(lba1_port, (Byte)(lbastart >> 8) & 0xff);
        outb(lba2_port, (Byte)(lbastart >> 16) & 0xff);

        outb(command_port, ATA_CMD_READ);
        ATASoftResetDelay(status_port);

        /* Again, wait for BSY to clear before checking for errors */
        Byte status;
        while ((status = inb(status_port)) & ATA_STATUS_BSY);

        /* Now it is safe to check for errors */
        if (status & ATA_STATUS_ERR) FatalError("Unable to read from disk index %d", index);

        Size n_read = 0;
        for (Size i = 0; i < n; i++) {
                if (WaitForDRQ(status_port) != STATUS_OK) return n_read;
                for (unsigned int j = 0; j < ATA_READ_BUFSIZE; j++)
                        ((u16 *)dest)[i * ATA_READ_BUFSIZE + j] = inw(dataport);
                n_read++;
        }
        return n_read;
}

Bool ATADriveIsATAPI(int index) { return devices[index].present && devices[index].atapi; }
