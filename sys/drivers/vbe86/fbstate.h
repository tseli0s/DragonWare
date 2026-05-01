/**********************************************************************
 * FILE: fbstate.h
 * PURPOSE: Framebuffer internal state
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "video/pixels.h"

typedef struct _FramebufferState {
        Size       column, row, pixels_per_char;
        Size       width, height, pitch;
        Byte       bpp;
        Bool       text_mode;
        PixelColor fg, bg;
        void      *addr;
} FramebufferState;
