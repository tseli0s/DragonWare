/**********************************************************************
 * FILE: mbutils.c
 * PURPOSE: Multiboot protocol utilities
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/
#include "mbutils.h"

#include <bits.h>

#include "proto/multiboot.h"
#include "textmode/dbgprint.h"

off_t FindMultibootHeader(u8 *multiboot_file) {
        off_t result = 0x123456;

        /* Per multiboot spec, the header MUST be aligned to four bytes. Otherwise this code won't
         * work. */
        for (Size i = 0; i < MULTIBOOT_SEARCH_FOR; i += MULTIBOOT_HEADER_ALIGN) {
                /* Casting to u32 here for easier pointer arithmetic */
                u32 *header = (u32 *)&multiboot_file[i];
                u32  magic  = header[0];

                if (magic == MULTIBOOT_HEADER_MAGIC) {
                        /* Okay, we've found something, but we must verify the checksum too */
                        u32 flags    = header[1];
                        u32 checksum = header[2];

                        /* Per spec:
                         * The field ‘checksum’ is a 32-bit unsigned value which,
                         * when added to the other magic fields (i.e. ‘magic’ and ‘flags’), must
                         * have a 32-bit unsigned sum of zero. */
                        u32 total = magic + flags;
                        if ((total + checksum) == 0) {
                                /* Okay, this is definitely a multiboot image, no need to keep
                                 * searching. */
                                result = i;
                                break;
                        } else {
                                DebugPrint(
                                        "Found a multiboot magic at %d, but checksum failed, "
                                        "ignoring.",
                                        i);
                        }
                }
        }

        return result;
}