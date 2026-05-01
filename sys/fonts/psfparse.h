/**********************************************************************
 * FILE: psfparse.h
 * PURPOSE: PSF font file parsing code for internal kernel use
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define PSF_FONT_MAGIC (0x0436)

#include <ktypes.h>

#include "macros.h"

typedef struct [[gnu::packed]] _PSFFont {
        u16 magic;
        u8  mode;
        u8  char_size;
} PSFFont;

typedef enum _PSFError { PSFGood = 0, BadHeader, BadVersion, BadOffset, BadDimensions } PSFError;

/* Parses a PSF file and checks that it's valid. Doesn't require memory allocation. */
PSFError ParsePSFData(const u8 *data);

/* Returns whether the given memory-mapped data is a valid PSF (1) font */
Bool FontISPSF(const u8 *data);
