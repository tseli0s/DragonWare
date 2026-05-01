/**********************************************************************
 * FILE: smbios.c
 * PURPOSE: SMBIOS support for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/
#include "smbios.h"

#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "log.h"
#include "paging.h"
#include "vmm.h"

/* Per spec */
#define SMBIOS_DATA_ALIGNMENT (16)

typedef struct [[gnu::packed]] _SMBIOSHeader {
        Byte type;
        Byte length;
        Word handle;
} SMBIOSHeader;

typedef struct [[gnu::packed]] _SMBIOSSystemInfo {
        SMBIOSHeader header;
        Byte         manufacturer_idx;
        Byte         product_name_idx;
        Byte         version_idx;
        Byte         serial_number_idx;
        char         uuid[16];
        Byte         wake_up_type;
        /* There are two more fields here but only for SMBIOS 2.4+ */
} SMBIOSSystemInfo;

static char *GetSMBIOSString(SMBIOSHeader *header, Byte index) {
        if (index == 0) return "Unknown";
        char *str = (char *)header + header->length;

        for (int i = 1; i < index; i++) {
                while (*str != '\0') str++;

                str++;

                if (*str == '\0') return "Bad Index / Unknown";
        }

        return str;
}

static SMBIOSSystemInfo *GetSMBIOSSystemInfo(SMBIOSData *entry_point, u16 count) {
        u8 *ptr = (u8 *)(uintptr_t)entry_point->structure_addr;

        for (int i = 0; i < count; i++) {
                SMBIOSHeader *header = (SMBIOSHeader *)ptr;
                if (header->type == 1) return (SMBIOSSystemInfo *)ptr;

                u8 *scan = ptr + header->length;

                while (!(scan[0] == 0 && scan[1] == 0)) scan++;
                ptr = scan + 2;
        }
        return NullPointer;
}

/**
 * @brief Verifies the checksum of the SMBIOS Entry Point.
 * The sum of all bytes in the structure must equal 0.
 */
[[gnu::nonnull(1)]]
static Bool IsSMBIOSChecksumValid(SMBIOSData *data) {
        u8 *bytes = (u8 *)data;
        u8  sum   = 0;

        for (u8 i = 0; i < data->entry_point_len; i++) {
                sum += bytes[i];
        }

        return (sum == 0);
}

uintptr_t FindSMBIOSDataArea(void) {
        /* Temporarily identity map the area to do our thing */
        MapMemoryRange(0xf0000, 0xf0000, PAGE_PRESENT, 16);
        uintptr_t addr = 0;
        for (Size current_addr = 0xf0000; current_addr <= 0xfffff;
             current_addr += SMBIOS_DATA_ALIGNMENT) {
                if (memcmp((void *)current_addr, "_SM_", 4) == 0) {
                        if (!IsSMBIOSChecksumValid((SMBIOSData *)current_addr))
                                continue; /* Checksum failed */
                        else {
                                LogMessage(LOG_DEBUG, "Found SMBIOS data at 0x%x", current_addr);
                                addr = current_addr;
                                break;
                        }
                }
        }

        if (addr == 0x0)
                LogMessage(LOG_WARNING,
                           "Unable to locate SMBIOS data area (System possibly unsupported)");
        UnmapMemoryRange(0xf0000, 16);
        return addr;
}

SMBIOSData *SMBIOSDataFromPointer(uintptr_t pointer) {
        if (pointer == 0) return NullPointer;

        Size        eps_structure = sizeof(SMBIOSData);
        SMBIOSData *data          = kmalloc(eps_structure);

        if (!data) return NullPointer;

        uintptr_t mapaddr = aligndown(pointer, PAGE_SIZE);
        MapSinglePage(mapaddr, mapaddr, PAGE_PRESENT);
        memcpy(data, (void *)pointer, eps_structure);
        UnmapSinglePage(mapaddr);
        return data;
}
void DumpSMBIOSData(SMBIOSData *data) {
        if (!data) return;

        LogMessage(LOG_INFO, "SMBIOS compatible firmware found, version %d.%d", data->major_version,
                   data->minor_version);
        uintptr_t phys_table = data->structure_addr;
        uintptr_t page_start = aligndown(phys_table, PAGE_SIZE);
        uintptr_t offset     = phys_table - page_start;
        Size      n_pages    = (offset + data->structure_size + PAGE_SIZE - 1) / PAGE_SIZE;

        MapMemoryRange(page_start, page_start, PAGE_PRESENT, n_pages);
        SMBIOSSystemInfo *info = GetSMBIOSSystemInfo(data, data->n_structures);

        if (info) {
                char *vendor  = GetSMBIOSString(&info->header, info->manufacturer_idx);
                char *product = GetSMBIOSString(&info->header, info->product_name_idx);

                LogMessage(LOG_INFO, "\tManufacturer: %s", vendor ? vendor : "Unknown");
                LogMessage(LOG_INFO, "\tProduct: %s", product ? product : "Unknown");
        } else {
                LogMessage(LOG_WARNING, "System Information structure (Type 1) not found.");
        }

        /* Cleanup the virtual mappings */
        for (Size i = 0; i < n_pages; i++) UnmapSinglePage(page_start + (i * PAGE_SIZE));
}
