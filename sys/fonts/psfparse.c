/**********************************************************************
 * FILE: psfparse.c
 * PURPOSE: PSF font file parsing code for internal kernel use
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "psfparse.h"

#include <log.h>
#include <mmutils.h>

#include "lib/assert.h"
#include "spleen.h"

PSFError ParsePSFData(const u8 *data) {
        PSFFont font = {0};
        memcpy(&font, data, sizeof(PSFFont));

        if (font.magic != PSF_FONT_MAGIC) return BadVersion;
        if (font.char_size != 16) return BadDimensions; /* We will require 8x16 for now */

        return PSFGood;
}

Bool FontISPSF(const u8 *data) {
        kassert(data != NullPointer);

        const PSFFont *f = (const PSFFont *)data;
        if (f->magic != PSF_FONT_MAGIC || f->char_size != 16) return false;

        if (f->magic != PSF_FONT_MAGIC)
                return false;
        else if (f->char_size != 16)
                return false;
        return true;
}
