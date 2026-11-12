/**********************************************************************
 * FILE: protocol.h
 * PURPOSE: Volume manager communication protocol
 * PROJECT: DragonWare Base System
 * DATE: 11-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>

/** @brief Volume manager protocol version 0 magic number constant */
#define VOLMGR_PROTOCOL_V0 ((u16)0xD4DA)

/**
 * @brief Data returned by the @ref VOLMGR_GET_VOLUME_INFO call.
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _VolumeManagerVolumeInfo {
        u64    volume_len;     /** << Length of the volume in sectors */
        u32    partition_type; /** << MBR partition type */
        Bool   active;         /** If true, this volume is the active one in the system*/
        Status result;      /** << Must be @ref STATUS_OK for the rest of the data to be valid. */
        Handle block_vol;   /** << Handle to the block device that handles the storage media access
                               underneath. Read/write requests must be sent to it. LBA is @b relative,
                               added to the starting LBA of the volume per request. */
        Byte   padding[11]; /* Just to align it to a power of 2 :P */
} VolumeManagerVolumeInfo;

/**
 * @brief Different kinds of requests that the volume manager in DragonWare understands.
 * @since v0.0.2
 */
typedef enum _VolumeManagerRequest : unsigned long {
        VOLMGR_NO_MESSAGE      = 0, /* catch bullshit requests easily, doxygen dont keep this  */
        VOLMGR_GET_VOLUME_INFO = 1, /** << Get information about a single volume */
        VOLMGR_CLAIM_VOLUME    = 2, /** << Claim this volume and accept future requests for it */
} VolumeManagerRequest;
