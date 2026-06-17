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
#include "textmode/dbgprint.h"

static Status GetVESAModeInformation(u16 mode, VBEModeInfo *info) {
        [[gnu::aligned(16)]]
        static VBEModeInfo local = {0};
        kzeromem(&local, sizeof(VBEModeInfo));

        uintptr_t     logicaladdr = (uintptr_t)&local;
        BIOSRegisters regs        = {.eax = VBE_GET_MODE_INFO,
                                     .ecx = mode & 0xFFFF,
                                     .es  = (logicaladdr >> 4) & 0xFFFF,
                                     .edi = (logicaladdr & 0xF)};
        if (BIOSCall(BIOS_VIDEO_SERVICES, &regs) != 0) return STATUS_BAD;
        if (regs.eax != VBE_SUCCESS) {
                DebugPrint(
                        "%s: EAX contained value 0x%x upon return! (Not matching VBE_SUCCESS "
                        "expected value)",
                        __func__, regs.eax);
                return STATUS_BAD;
        }
        memcpy(info, &local, sizeof(VBEModeInfo));
        return STATUS_OK;
}

Status GetVESAInformationBlock(VBEInfo *info) {
        /* This is required when calling on the BIOS. */
        char signature[] = "VBE2";
        memcpy(info->signature, signature, strlen(signature));

        uint32_t linear_addr = (uintptr_t)info;

        BIOSRegisters regs = {.eax = VBE_GET_BIOS_INFO};
        regs.es            = (linear_addr >> 4) & 0xFFFF;
        regs.edi           = (linear_addr & 0xF);
        if (BIOSCall(BIOS_VIDEO_SERVICES, &regs) != 0) {
                FatalError(
                        "Video services requested possibly unsupported by this card (Carry flag "
                        "set in EFLAGS).");
        }

        if (regs.eax != VBE_SUCCESS) {
                FatalError("VESA BIOS Extensions fetching failed (EAX=0x%x)", regs.eax);
        }
        return STATUS_OK;
}

/*
 * I am not sure if this handles all edge cases. Hopefully it does.
 * I tried to get inspiration from other bootloaders, but they have a lot of
 * different ways to determine "best mode" with different checks, hardware quirks
 * and so on. So I just sum up the mode parameters and find the difference with the
 * desired mode sum, the closest to 0, the better.
 */
Status FindBestVESAMode(VBEInfo *info, VBEModeInfo *modeinfo, int w, int h, int d, u16 *mode) {
        /* default mode, in case it was not specified */
        if (!w) w = 1024;
        if (!h) h = 768;
        if (!d) d = 32;

        u16        *list          = (u16 *)SegmentedToLinearAddress(info->modelist);
        u16         mode_selected = 0;
        VBEModeInfo mi;
        int         i           = 0;
        int         last_result = 0;

        while (list[i] != 0xFFFF) {
                if (GetVESAModeInformation(list[i], &mi) != STATUS_OK) return STATUS_BAD;
                int target_score = (w * h) + d;
                int this_score   = (mi.width * mi.height) + mi.bpp;
                int result       = target_score - this_score;

                if (result == 0) {
                        *mode = list[i];
                        memcpy(modeinfo, &mi, sizeof(VBEModeInfo));
                        return STATUS_OK;
                }

                else if (last_result < result) {
                        mode_selected = list[i];
                        last_result   = result;
                }
                i++;
        }

        *mode = mode_selected;
        memcpy(modeinfo, &mi, sizeof(VBEModeInfo));
        return STATUS_NOT_FOUND;
}

Status VESAModeset(u16 mode) {
        if (mode == 0xFFFF) return STATUS_BAD_ARGUMENT;
        BIOSRegisters r = {
                .eax = VBE_SET_VIDEO_MODE,
                .ebx = mode | VESA_LINEAR_FB,
        };
        if (BIOSCall(BIOS_VIDEO_SERVICES, &r) != 0) return STATUS_BAD;
        if (r.eax != VBE_SUCCESS) return STATUS_BAD;

        DebugPrint("Switched to linear framebuffer mode %d", mode);
        return STATUS_OK;
}
