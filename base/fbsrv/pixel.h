/**********************************************************************
 * FILE: pixel.h
 * PURPOSE: Pixel manipulation utilities for the framebuffer server
 * PROJECT: DragonWare Base System
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* TODO: This assumes column-major order, so x and y in this formula must be swapped otherwise. */
#define PIXEL_INDEX(_x, _y, _fbwidth) (_y * _fbwidth + _x)

#include <kerneltypes.h>

static inline u32 Color24To32(u32 color_24bit) { return (color_24bit | 0xFF); }

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
                              u32 clr) {
        u8 *row        = addr + y * pitch;
        u32 color      = Color24To32(clr);
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
