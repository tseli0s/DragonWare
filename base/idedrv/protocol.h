/**********************************************************************
 * FILE: protocol.h
 * PURPOSE: DragonWare block driver implementation protocol exports
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/* Quick note (12/11/2026): This is not a header specific to idedrv. It is meant to be a protocol
 * supported by all disk drivers, to make writing filesystem drivers significantly easier.*/

#pragma once

#include <kerneltypes.h>
#include <object.h>

/* LBA type definition */
typedef u64 LBA;

/* Protocol version 0 */
#define BLOCKDRV_PROTOCOL_V0 ((u16)0xB10C)

/* Status codes returned by a request to any block driver. */
typedef enum BlockDriverStatusReply : u32 {
        BLOCKDRV_SUCCESS = 0,      /* Everything went well */
        BLOCKDRV_ACCESS_DENIED,    /* Access to this process was denied */
        BLOCKDRV_HARDWARE_FAILURE, /* Hard drive is failing or bad sector request */
        BLOCKDRV_INVALID_HANDLE,   /* Invalid handle given */
        BLOCKDRV_BAD_PARAMETER,    /* Bad parameter in IPC message */
        BLOCKDRV_OUT_OF_MEMORY,    /* Out of memory (Kernel can't share the memory between the
                                    two processes) */
        BLOCKDRV_LOCKED,    /* Access to the drive is temporarily forbidden (eg contention )*/
        BLOCKDRV_BUG_CHECK, /* Internal driver bug */
} BlockDriverStatusReply;

/**
 * @brief An opcode that a disk driver usually implements.
 * @since v0.0.2
 */
typedef enum _BlockDriverRequestOpcode : unsigned long {
        BLOCKDRV_BAD_REQUEST = 0, /* again, just tryna catch bad applications */
        BLOCKDRV_READ_SECTOR,     /** << Read a single sector into shared memory */
        BLOCKDRV_WRITE_SECTOR, /** << Write a single sector, copied from a shared memory buffer */
        BLOCKDRV_GET_DRIVE_PARAMS = 0xff /** @warning @b RESERVED (Do not use this opcode) */
} BlockDriverRequestOpcode;

/* Simply memcpy() this struct into the message payload when sending it */
typedef struct [[gnu::packed]] BlockDriverRequest {
        LBA    lba;            /* LBA number to perform the request to */
        Handle shared_section; /* Handle to the section to be shared between caller and callee and
                                  read/write the data from/to. */
        int    master;         /* 0 for the master drive on the bus, 1 for the slave drive */
        int    __reserved;     /* Never used for now */
} BlockDriverRequest;

typedef struct [[gnu::packed]] BlockDriverReplyData {
        BlockDriverStatusReply reply; /* Reply to the request given */
} BlockDriverReplyData;
