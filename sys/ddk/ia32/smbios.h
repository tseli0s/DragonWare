/**********************************************************************
 * FILE: smbios.h
 * PURPOSE: SMBIOS support for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _SMBIOSData {
        char id[4];             /* _SM_ */
        Byte checksum;          /* Checksum of the entry point structure. */
        Byte entry_point_len;   /* Entry point length. Currently 0x1f. */
        Byte major_version;     /* Major version of this system's SMBIOS */
        Byte minor_version;     /* Minor >> */
        u16  maximum_data_size; /* Maximum data size in this structure, unused by us */
        Byte revision;          /* Must be 0 if this is SMBIOS v2.1 */
        Byte formatted[5];      /* Reserved if revision is 0 */
        char dmi[5];            /* Must be _DMI_ */
        Byte checksum_interm;   /* Intermediate checksum. */
        u16  structure_size;    /* Total size of the SMBIOS structure table */
        u32  structure_addr;    /* Read-only address of the SMBIOS structure table */
        u16  n_structures; /* Total number of structures present in the SMBIOS structure table */
        Byte bcd_revision; /* Uh too long to write here just ignore it */
} SMBIOSData;

/**
 * @brief Find the SMBIOS data by scanning in the 0xf0000-0xfffff range for the _SM_ string.
 * @note See the specification here:
 * https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.9.0.pdf
 * @return The address of the SMBIOS data collection, or 0 if it could not be found.
 */
uintptr_t FindSMBIOSDataArea(void);

/**
 * @brief Copies SMBIOS data from the pointer given, usually the pointer being returned from @ref
 * FindSMBIOSDataArea
 * @param pointer The pointer at the SMBIOS v2.1 address, fetched using @ref FindSMBIOSDataArea
 * @return A heap allocated @ref SMBIOSData structure copied from @p pointer
 */
SMBIOSData* SMBIOSDataFromPointer(uintptr_t pointer);

void DumpSMBIOSData(SMBIOSData* data);
