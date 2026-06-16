/**********************************************************************
 * FILE: console.c
 * PURPOSE: Console service implemented on top of the framebuffer driver
 * PROJECT: DragonWare Base System
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/*
 * MAJOR FIXME IN THIS FILE: Currently, all functions here assume a depth of 32 bits, without the
 * ability to use 24, 16 (much less 40+) bits to render. This *will* be broken in a lot of hardware,
 * so this is one of the things that should be addressed here first. At the moment, it also assumes
 * a fixed resolution of 1024x768 pixels, so if the bootloader chooses literally any other
 * resolution, this will look broken and cause crashes.
 */

#include "console.h"

#include <ipc86.h>

#include "font/glyph.h"
#include "protocol.h"

/* Vertical position we are currently at */
static int xpos = 0;

/* Horizontal position we are currently at. */
static int ypos = 0;

static inline void PutPixel32(u32 *addr, unsigned long fbwidth, unsigned long x, unsigned long y,
                              u32 color) {
        Size idx  = PIXEL_INDEX(x, y, fbwidth);
        addr[idx] = color;  // NOLINT(clang-analyzer-core.FixedAddressDereference)
        /* (that NOLINT line above is because clang-tidy thinks we're doing something stupid )*/
}

[[gnu::hot]]
static void WriteSinglePixel(int x, int y, u32 rgba) {}

[[gnu::hot]]
static void DrawCharacterAuto(char c) {}

void HandleConsoleClientRequest(Message *m) {
        switch (m->header.type) {
                default:
                        break;
        }
}
