/**********************************************************************
 * FILE: protocol.h
 * PURPOSE: DragonWare ATA/IDE driver protocol exports
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>
#include <object.h>

/* LBA type definition */
typedef u64 LBA;

/* Protocol version 0 */
#define IDEDRV_PROTOCOL_V0 ((u16)0x1DEA)

/* Status codes returned by a request to idedrv. */
typedef enum __IDEDRVStatusReply
        : u32 { IDEDRV_SUCCESS = 0,      /* Everything went well */
                IDEDRV_ACCESS_DENIED,    /* Access to this process was denied */
                IDEDRV_HARDWARE_FAILURE, /* Hard drive is failing or bad sector request */
                IDEDRV_INVALID_HANDLE,   /* Invalid handle given */
                IDEDRV_BAD_PARAMETER,    /* Bad parameter in IPC message */
                IDEDRV_OUT_OF_MEMORY,    /* Out of memory (Kernel can't share the memory between the
                                            two processes) */
                IDEDRV_LOCKED, /* Access to the drive is temporarily forbidden (eg contention )*/
        } IDEDRVStatusReply;

/* Simply memcpy() this struct into the message payload when sending it */
typedef struct [[gnu::packed]] __IDEDRVRequest {
        LBA lba; /* LBA number to perform the request to */
        Handle shared_section; /* Handle to the section to be shared between caller and callee and
                                  read/write the data from/to. */
        int master; /* 0 for the master drive on the bus, 1 for the slave drive */
        int __reserved; /* Never used for now */
} IDEDRVRequest;

typedef struct [[gnu::packed]] __IDEDRVReplyData {
        IDEDRVStatusReply reply; /* Reply to the request given */
} IDEDRVReplyData;

/*
 * Message type: Read single sector into memory
 * Message header reply handle must point to a port to send the status of this request.
 */
#define IDEDRV_READ_SECTOR      (0x01)

/*
 * Message type: Write single sector to disk from memory
 *
 * Message header reply handle must point to a port to send the status of this request. The section
 * bytes 0-511 must contain the data that will be written. Payload contents:
 * - Bytes 0-7: Amount of bytes to write.
 */
#define IDEDRV_WRITE_SECTOR     (0x02)

/* RESERVED: DO NOT USE */
#define IDEDRV_GET_DRIVE_PARAMS (0xFF)
