/**********************************************************************
 * FILE: glyph.h
 * PURPOSE: Glyph rendering exports for internal kernel use
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define FONT_WIDTH  (8)
#define FONT_HEIGHT (16)

#ifdef __DRAGONWARE_SYS__ /* Only defined for the kernel build, not for the userland */
#include <ktypes.h>
#else
#include <kerneltypes.h>
#endif /* __DRAGONWARE_SYS__ */


typedef struct _Glyph {
        u8  *font;
        u8   charsize;
        Size offset;
} Glyph;

/* Returns the offset of the raster glyph in a font */
Glyph GetGlyphFromDefaultFont(char c);
