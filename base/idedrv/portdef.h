/**********************************************************************
 * FILE: portdef.h
 * PURPOSE: I/O port number defines header
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

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

#define ATA_STATUS_ERROR (0x01)
#define ATA_STATUS_DRQ (0x02)
#define ATA_STATUS_DF (0x08)
#define ATA_STATUS_RDY (0x10)
#define ATA_STATUS_BSY (0x20)
