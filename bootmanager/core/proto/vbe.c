/**********************************************************************
 * FILE: vbe.c
 * PURPOSE: VESA BIOS Extensions helper functions
 * PROJECT: DragonWare Boot Manager
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "vbe.h"

#include <kstring.h>
#include <ktypes.h>
#include <mmutils.h>

#include "cpu/bioscall.h"
#include "error.h"

Status GetVESAInformationBlock(VBEInfo *info) {
        VBEInfo infoblock;
        /* This is required when calling on the BIOS. */
        char signature[] = "VBE2";
        memcpy(infoblock.signature, signature, strlen(signature));
        BIOSRegisters regs = {.eax = VBE_GET_BIOS_INFO,
                              .es  = SEGMENT_OF(infoblock),
                              .edi = OFFSET_IN_SEGMENT_OF(infoblock)};
        if (BIOSCall(BIOS_VIDEO_SERVICES, &regs) != 0)
                FatalError(
                        "Video services requested possibly unsupported by this card (Carry flag "
                        "set in EFLAGS).");

        if (regs.eax != VBE_SUCCESS)
                FatalError("VESA BIOS Extensions fetching failed (EAX=0x%x)", regs.eax);

        char returned_signature[] = "VESA";
        if (memcmp(infoblock.signature, returned_signature, sizeof(infoblock.signature)) != 0) {
                u32 signature_in_u32;
                memcpy(&signature_in_u32, returned_signature, sizeof(u32));

                u32 vesa_in_u32;
                memcpy(&vesa_in_u32, "VESA", sizeof(u32));

                FatalError("Signature mismatch in VBE information fetch call: 0x%x against 0x%x",
                           signature_in_u32, vesa_in_u32);
        }

        memcpy(info, &infoblock, sizeof(VBEInfo));
        return STATUS_OK;
}
