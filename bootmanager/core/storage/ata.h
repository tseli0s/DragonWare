/**********************************************************************
 * FILE: ata.h
 * PURPOSE: ATA/IDE support code implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#define MAX_ATA_DISKS        (4)
#define MAX_SECTORS_PER_READ (255)

typedef enum _ATAError {
        ATA_ERR_NONE  = 0,
        ATA_ERR_NODEV = -1,
        ATA_ERR_ATAPI = -2,
        ATA_DEVERR    = -3
} ATAError;

typedef struct [[gnu::packed]] _ATAIdentifyData {
        u16  unused[10];
        char serial[20];
        u16  unused2[3];
        char firmware[8];
        char model[40];
        u16  unused3[13];
        u32  lba28_sectors;
        u16  unused4[38];
        u64  lba48_sectors;
} ATAIdentifyData;

static inline const char *ATAErrorCodeToString(ATAError e) {
        switch (e) {
                case ATA_ERR_NODEV:
                        return "Device does not exist";
                case ATA_ERR_ATAPI:
                        return "Device is CD/DVD (ATAPI)";
                case ATA_ERR_NONE:
                default:
                        return "No Error";
        }
}

/**
 * @brief Returns the amount
 *
 * @param max_devices
 * @return int
 */
int ATAIdentifyAllDevices(int max_devices);

/**
 * @brief Returns whether the given drive is present in the computer or not, based on the ATA
 * interface.
 * @ref ATAIdentifyAllDevices must be called first to register the device.
 * @p index meaning:
 * 0 = master primary drive
 * 1 = slave primary drive
 * 2 = master secondary drive
 * 3 = slave secondary drive
 * @param index The index of the drive to check if it is present or not.
 * @returns true if the device is present and can be read from, false otherwise.
 */
Bool ATADriveIsPresent(int index);

/**
 * @brief Reads bytes from the given ATA drive into @p dest, starting from LBA @p lbastart.
 * @param[in] index meaning:\
 * 0 = master primary drive
 * 1 = slave primary drive
 * 2 = master secondary drive
 * 3 = slave secondary drive
 * @param[in] n Amount of sectors to read. Each sector = 512 bytes (other sector sizes are emulated
 * in software to be 512).
 * @param[in] lbastart From which LBA to start reading. Only LBA28 addressing is supported.
 * @param[out] dest Destination buffer to write the data to.
 * @returns The amount of sectors read. Assumes each sector = 512 bytes. 0 if there was an error.
 * @sa ATADriveIsPresent
 */
Size ATAReadSectors(int index, Size n, u32 lbastart, Byte *dest);

/**
 * @brief Returns whether the drive @p index given is an ATAPI device (eg. A CD-ROM)
 * @param[in] index Index of the drive, see documentation of other functions for more details.
 * @sa ATADriveIsPresent
 * @return Bool
 */
Bool ATADriveIsATAPI(int index);
