/**********************************************************************
 * FILE: protocol.h
 * PURPOSE: DragonWare ATA/IDE driver protocol exports
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* Protocol version 0 */
#define IDEDRV_PROTOCOL_V0 ((u16)0x1DEA)

/*
 * Message type: Read single sector into memory
 *
 * Message header reply handle must point to a section object that will be shared between the caller
 * and this driver. Payload contents:
 * - Bytes 0-3: LBA number to read from
 * - Byte 4: 0 for primary bus, 1 for secondary bus, other values will be interpeted as primary.
 * - Byte 5: 0 for master drive, 1 for slave drive,  other values will be interpeted as master.
 */
#define IDEDRV_READ_SECTOR (0x01)

/*
 * Message type: Write single sector to disk from memory
 *
 * Message header reply handle must point to a section object that will be shared between the caller
 * and this driver. The section bytes 0-511 must contain the data that will be written. Payload contents:
 * - Bytes 0-3: LBA number to write to
 * - Bytes 4-11: Amount of bytes to write.
 * - Byte 12: 0 for primary bus, 1 for secondary bus, other values will be interpeted as primary.
 * - Byte 13: 0 for master drive, 1 for slave drive,  other values will be interpeted as master.
 */
#define IDEDRV_WRITE_SECTOR (0x02)

/* RESERVED: DO NOT USE */
#define IDEDRV_GET_DRIVE_PARAMS (0xFF)
