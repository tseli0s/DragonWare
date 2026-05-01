/**********************************************************************
 * FILE: vgatext.h
 * PURPOSE: VGA text mode implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#define VGA_WIDTH         (80)
#define VGA_HEIGHT        (25)
#define VGA_ADDR          (0xB8000)

/* Returns the start of the nth column given */
#define N_COL_START(n)    ((n) * VGA_WIDTH)
#define DEFAULT_VGA_COLOR (VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY, VGATEXT_COLOR_BLACK))

typedef enum _VGAColor : unsigned char {
        VGATEXT_COLOR_BLACK         = 0,
        VGATEXT_COLOR_BLUE          = 1,
        VGATEXT_COLOR_GREEN         = 2,
        VGATEXT_COLOR_CYAN          = 3,
        VGATEXT_COLOR_RED           = 4,
        VGATEXT_COLOR_MAGENTA       = 5,
        VGATEXT_COLOR_BROWN         = 6,
        VGATEXT_COLOR_LIGHT_GREY    = 7,
        VGATEXT_COLOR_DARK_GREY     = 8,
        VGATEXT_COLOR_LIGHT_BLUE    = 9,
        VGATEXT_COLOR_LIGHT_GREEN   = 10,
        VGATEXT_COLOR_LIGHT_CYAN    = 11,
        VGATEXT_COLOR_LIGHT_RED     = 12,
        VGATEXT_COLOR_LIGHT_MAGENTA = 13,
        VGATEXT_COLOR_LIGHT_BROWN   = 14,
        VGATEXT_COLOR_WHITE         = 15,
} VGAColor;

static inline Byte VGAGetColorAttribute(VGAColor fg, VGAColor bg) {
        return (Byte)(fg | bg << (Byte)4);
}

void VGATextInit(void);
void VGAPrintCharAt(int x, int y, char c, VGAColor color);
void VGAPrintString(const char *str, VGAColor color);
void VGAPrintStringAt(int x, int y, const char *str, VGAColor color);
void VGAPrintCenteredString(int y, const char *str, VGAColor color);
void VGAClearAllText(Byte colorattr);
