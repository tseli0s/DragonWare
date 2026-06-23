/**********************************************************************
 * FILE: psfparse.c
 * PURPOSE: PSF font file parsing code for internal kernel use
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "psfparse.h"

#ifdef __DRAGONWARE_SYS__ /* Only defined for the kernel build, not for the userland */
#include <ktypes.h>
#include <mmutils.h>
#else
#include <kerneltypes.h>
#include <string.h>
#endif /* __DRAGONWARE_SYS__ */

Status ParsePSFData(const u8 *data) {
        PSFFont font = {0};
        memcpy(&font, data, sizeof(PSFFont));

        if (font.magic != PSF_FONT_MAGIC) return STATUS_UNSUPPORTED;
        if (font.char_size != 16) return STATUS_BAD_ARGUMENT; /* We will require 8x16 for now */

        return STATUS_OK;
}

Bool FontISPSF(const u8 *data) {
        const PSFFont *f = (const PSFFont *)data;
        if (f->magic != PSF_FONT_MAGIC || f->char_size != 16) return false;

        if (f->magic != PSF_FONT_MAGIC)
                return false;
        else if (f->char_size != 16)
                return false;
        return true;
}
