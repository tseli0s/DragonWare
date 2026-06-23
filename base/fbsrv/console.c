/**********************************************************************
 * FILE: console.c
 * PURPOSE: Framebuffer-based console implementation
 * PROJECT: DragonWare Base System
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <string.h>
#define BLACK_COLOR (0x00000000)
#define WHITE_COLOR (0xFFFFFFFF)

#include <kerneltypes.h>

#include "console.h"
#include "font/glyph.h"
#include "pixel.h"

/* Character coordinates. Sorry if the name sucks, I like single character variables for this. */
static int x = 0;
static int y = 0;

static struct {
        u32 w, h, d;
        u32 stride;
} info;

[[gnu::hot]]
static void WriteSinglePixel(u32 xc, u32 yc, u32 color) {
        switch (info.d) {
                case 32: {
                        PutPixel32(FRAMEBUFFER_ADDR, info.w, xc, yc, color);
                        break;
                }
                case 24: {
                        PutPixel24(FRAMEBUFFER_ADDR, info.stride, xc, yc, color);
                        break;
                }

                default:
                        /* Just assume 32 bits for now */
                        PutPixel32(FRAMEBUFFER_ADDR, info.w, xc, yc, color);
                        break;
        }
}

[[gnu::hot]]
static void RenderGlyph(int xc, int yc, Glyph *g) {
        u8 *glyph = g->font + g->offset;

        for (u32 row = 0; row < FONT_HEIGHT; row++) {
                u8 bits = glyph[row];

                for (u32 col = 0; col < FONT_WIDTH; col++) {
                        u32 px = (u32)xc + col;
                        u32 py = (u32)yc + row;

                        if (px >= info.w || py >= info.h) continue;

                        u32 color = (bits & (0x80 >> col)) ? WHITE_COLOR : BLACK_COLOR;
                        WriteSinglePixel(px, py, color);
                }
        }
}

[[gnu::hot]]
static inline void WriteSingleCharacterAt(char c, int xc, int yc) {
        const Glyph g = GetGlyphFromDefaultFont(c);
        RenderGlyph(xc * FONT_WIDTH, yc * FONT_HEIGHT, (Glyph *)&g);
}

[[gnu::hot]]
static void ScrollFramebuffer(void) {
        memmove(FRAMEBUFFER_ADDR, (u8 *)FRAMEBUFFER_ADDR + (FONT_HEIGHT * info.stride),
                (info.h - FONT_HEIGHT) * info.stride);

        int last_row = (info.h / FONT_HEIGHT) - 1;
        int max_cols = info.w / FONT_WIDTH;

        for (int i = 0; i < max_cols; i++) WriteSingleCharacterAt(' ', i, last_row);

        x = 0;
        y = last_row;
}

static inline void ClearFramebuffer(void) { memset(FRAMEBUFFER_ADDR, 0x00, info.h * info.stride); }

void RegisterDeviceInfo(DeviceMapDescriptor *dev) {
        info.w      = dev->fb.width;
        info.h      = dev->fb.height;
        info.d      = dev->fb.bpp;
        info.stride = dev->fb.stride;
        ClearFramebuffer();
}

void WriteCharacterToConsole(char c) {
        int max_cols = (int)(info.w / FONT_WIDTH) - 2;

        if (c == '\n') {
                x = 0;
                y++;
                if ((u32)y >= (info.h / FONT_HEIGHT) - 1) ScrollFramebuffer();
                return;
        } else if (c == '\b') {
                if (x > 0)
                        x--;
                else if (y > 0) {
                        y--;
                        x = (info.w / FONT_WIDTH) - 1;
                }
                WriteSingleCharacterAt(' ', x, y);
                return;
        } else if (c == '\t') {
                x = (int)(((unsigned int)x + 4) & (Size)~3);
                return;
        }
        WriteSingleCharacterAt(c, x, y);
        x++;
        if (x >= max_cols) {
                x = 0;
                y++;
        }
}

void WriteStringToConsole(const char *str) {
        size_t len = strlen(str);
        for (size_t i = 0; i < len; i++) {
                WriteCharacterToConsole(str[i]);
        }
}
