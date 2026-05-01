/**********************************************************************
 * FILE: glyph.c
 * PURPOSE: Glyph rendering support
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "glyph.h"

#include "macros.h"
#include "psfparse.h"
#include "spleen.h"

#define GLYPH_OFFSET(_char, _charsize) (sizeof(PSFFont) + (Size)(_char) * (Size)(_charsize))

Glyph GetGlyphFromDefaultFont(char c) {
        Glyph g    = {0};
        g.font     = (u8 *)_kernel_font;
        g.charsize = 16;
        g.offset   = GLYPH_OFFSET(c, 16);

        if (unlikely(!FontISPSF(g.font))) g.font = NullPointer;
        return g;
}
