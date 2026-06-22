/**********************************************************************
 * FILE: diskread.h
 * PURPOSE: BIOS-based disk access implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define SECTOR_SIZE    (512)
#define CD_SECTOR_SIZE (2048)

#include <ktypes.h>

/**
 * @brief Read from a storage medium using the BIOS and store the read content in @p dest
 * @param drive The BIOS drive to read from. Usually 0x80 for the first hard drive on the system.
 * @param lba LBA to start reading from.
 * @param n_sectors Amount of sectors to read. Each drive has a different sector size, be wary of
 * buffer overflows!
 * @param[out] dest Where to store the read content. Must not be @ref NullPointer.
 * @return STATUS_OK if the read was successful, other error codes in case of failure.
 */
[[gnu::nonnull]]
Status ReadFromDisk(int drive, u64 lba, int n_sectors, void *dest);
