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

#ifdef __DRAGONWARE_SYS__ /* Only defined for the kernel build, not for the userland */
#include <ktypes.h>
#else
#include <kerneltypes.h>
#endif /* __DRAGONWARE_SYS__ */

typedef struct [[gnu::packed]] _PSFFont {
        u16 magic;
        u8  mode;
        u8  char_size;
} PSFFont;

/* Parses a PSF file and checks that it's valid. Doesn't require memory allocation. */
[[gnu::nonnull]]
Status ParsePSFData(const u8 *data);

/* Returns whether the given memory-mapped data is a valid PSF (1) font */
[[gnu::nonnull]]
Bool FontISPSF(const u8 *data);
