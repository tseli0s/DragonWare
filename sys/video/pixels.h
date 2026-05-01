/**********************************************************************
 * FILE: pixels.h
 * PURPOSE: Pixel color abstraction and conversion between formats
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* Generic pixel plotting functions implementation, designed to have as little overhead as possible.
 * Though try not calling them in loops :)
 */

/* TODO: This assumes column-major order, so x and y in this formula must be swapped otherwise. */
#define PIXEL_INDEX(_x, _y, _fbwidth) (_y * _fbwidth + _x)
#define AREA_2D(_x, _y)               (_x * _y)

/** @brief A single pixel's color to be used with pixel manipulation/plotting functions. */
typedef enum _PixelColor {
        BlackPixel,
        WhitePixel,
        RedPixel,
        GreenPixel,
        BluePixel,
        LightBluePixel,
        YellowPixel,
        OrangePixel,
        MagentaPixel,
        CyanPixel,
        LightGrayPixel,
        PrussianPixel
} PixelColor;

/**
 * @brief Writes a single color value to the given 8-bit framebuffer, at the given location.
 * @param addr The framebuffer address that will be written to. Usually @ref FRAMEBUFFER_ADDR
 * @param fbwidth The width of the framebuffer, in pixels.
 * @param x The x (horizontal) position, in pixels, that the new pixel will be placed in
 * @param y The y (vertical) position, in pixels, that the new pixel will be placed in
 * @param color An 8 bit value of the color to write (Format may vary, see warning below)
 * @warning This function probably doesn't work as expected unless each vendor is handled specially.
 */
static inline void PutPixel8(u8 *addr, unsigned long fbwidth, unsigned long x, unsigned long y,
                             u8 color) {
        Size idx  = PIXEL_INDEX(x, y, fbwidth);
        addr[idx] = color;
}

/**
 * @brief Writes a single color value to the given 16-bit framebuffer, at the given location.
 * @param addr The framebuffer address that will be written to. Usually @ref FRAMEBUFFER_ADDR
 * @param fbwidth The width of the framebuffer, in pixels.
 * @param x The x (horizontal) position, in pixels, that the new pixel will be placed in
 * @param y The y (vertical) position, in pixels, that the new pixel will be placed in
 * @param color A 16 bit value of the color to write (Format may vary, see warning below)
 * @warning This function probably doesn't work as expected unless each vendor is handled specially.
 */
static inline void PutPixel16(u16 *addr, unsigned long fbwidth, unsigned long x, unsigned long y,
                              u16 color) {
        Size idx  = PIXEL_INDEX(x, y, fbwidth);
        addr[idx] = color;
}

/**
 * @brief Returns a pixel's color based on a given @ref PixelColor as a 32 bit value (the higher
 * four bits are ignored).
 * @param p The pixel color that must be converted.
 * @returns The 32 bit integer version of the given @ref PixelColor in RGB format, or 0x00000000 if
 * the color couldn't be matched.
 * @warning Blitting with non-powers of two is very expensive to the CPU.
 */
static inline u32 ColorToBytes24(PixelColor p) {
        switch (p) {
                case BlackPixel:
                        return 0x00000000;
                case WhitePixel:
                        return 0x00FFFFFF - 1;
                case RedPixel:
                        return 0x00FF0000;
                case GreenPixel:
                        return 0x0000FF00;
                case BluePixel:
                        return 0x00000088;
                case LightBluePixel:
                        return 0x000077FF;
                case YellowPixel:
                        return 0x00FFFF00;
                case OrangePixel:
                        return 0x00FF9900;
                case MagentaPixel:
                        return 0x00FF00FF;
                case CyanPixel:
                        return 0x0000DDFF;
                case LightGrayPixel:
                        return 0x00333333;
                case PrussianPixel:
                        return 0x00010410;
                default:
                        return ColorToBytes24(BlackPixel);
        }
}

/**
 * @brief Writes a single color value to the given 24-bit framebuffer, at the given location.
 * @param addr The framebuffer address that will be written to. Usually @ref FRAMEBUFFER_ADDR
 * @param pitch The pitch (Bytes per scanline) of the framebuffer, see @ref VBEModeInfo
 * @param x The x (horizontal) position, in pixels, that the new pixel will be placed in
 * @param y The y (vertical) position, in pixels, that the new pixel will be placed in
 * @param color A 32 bit value of the color to write (RGBA format), usually returned by @ref
 * ColorToBytes24
 * @warning This function is very slow compared to @ref PutPixel32. Every pixel must be copied
 * separately as an 8 bit value.
 */
static inline void PutPixel24(u8 *addr, unsigned long pitch, unsigned long x, unsigned long y,
                              PixelColor clr) {
        u8 *row        = addr + y * pitch;
        u32 color      = ColorToBytes24(clr);
        row[x * 3 + 0] = color & 0xFF;
        row[x * 3 + 1] = (color >> 8) & 0xFF;
        row[x * 3 + 2] = (color >> 16) & 0xFF;
}

/**
 * @brief Writes a single 32 bit pixel value to the given framebuffer, at the given location.
 * @param addr The framebuffer address that will be written to. Usually @ref FRAMEBUFFER_ADDR
 * @param fbwidth The width of the framebuffer in pixels
 * @param x The x (horizontal) position, in pixels, that the new pixel will be placed in
 * @param y The y (vertical) position, in pixels, that the new pixel will be placed in
 * @param color A 32 bit value of the color to write (RGBA format), usually returned by @ref
 * ColorToBytes32
 */
static inline void PutPixel32(u32 *addr, unsigned long fbwidth, unsigned long x, unsigned long y,
                              u32 color) {
        Size idx  = PIXEL_INDEX(x, y, fbwidth);
        addr[idx] = color;  // NOLINT(clang-analyzer-core.FixedAddressDereference)
        /* (that NOLINT line above is because clang-tidy thinks we're doing something stupid )*/
}

/**
 * @brief Returns a pixel's color based on a given @ref PixelColor as a 32 bit value.
 * @param p The pixel color that must be converted.
 * @returns The 32 bit integer version of the given @ref PixelColor in RGBA format, or 0x00000000 if
 * the color couldn't be matched.
 */
static inline u32 ColorToBytes32(PixelColor p) {
        switch (p) {
                case BlackPixel:
                        return 0x00000000;
                case WhitePixel:
                        return 0xFFFFFFFF;
                case RedPixel:
                        return 0xFFFF0000;
                case GreenPixel:
                        return 0xFF00FF00;
                case BluePixel:
                        return 0xFF0000FF;
                case LightBluePixel:
                        return 0xFF0077FF;
                case YellowPixel:
                        return 0xFFFF00FF;
                case OrangePixel:
                        return 0xFFFF9900;
                case MagentaPixel:
                        return 0xFF00FFFF;
                case CyanPixel:
                        return 0xFF00DDFF;
                case LightGrayPixel:
                        return 0x11111111;
                case PrussianPixel:
                        return 0xFF010410;
                default:
                        return ColorToBytes32(BlackPixel);
        }
}
