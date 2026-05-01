/**********************************************************************
 * FILE: vbe.h
 * PURPOSE: VESA BIOS extensions definitions
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#define VBE_FLAG_SWITCHABLE_DAC    (0x1) /* Whatever the fuck that means */
#define VBE_ADAPTER_VGA_COMPATIBLE (0x2)
#define VBE_RAMDAC_FIX             (0x4) /* Literally what the fuck... ion care lmao */

/**
 * @brief A structure containing vendor information for a VBE-compatible graphics card.
 * @sa VBEModeInfo
 */
typedef struct [[gnu::packed]] _VBEInfo {
        u8    signature[4];
        u16   version;
        char *oem;
        u32   capabilities;
        u32   modelist;
        u16   totalmem; /* x64 kilobytes each, only valid in VBE 2.0+ */
        u16   revision;
        char *vendorname;
        char *productname;
        char *productrevision;
} VBEInfo;

/**
 * @brief A descriptor for a single VBE mode, selected either by the bootloader or the
 * int 0x10 BIOS functions.
 */
typedef struct [[gnu::packed]] _VBEModeInfo {
        u16 attributes;
        u8  window_a, window_b;
        u16 granularity;
        u16 window_size;
        u16 segment_a;
        u16 segment_b;
        u32 win_func_ptr;
        u16 pitch;
        u16 width, height;
        u8  w_char, y_char;
        u8  planes;
        u8  bpp;
        u8  banks;
        u8  memory_model;
        u8  bank_size;
        u8  image_pages;
        u8  reserved0;

        u8 red_mask;
        u8 red_position;
        u8 green_mask;
        u8 green_position;
        u8 blue_mask;
        u8 blue_position;
        u8 reserved_mask;
        u8 reserved_position;
        u8 direct_color_attributes;

        u32 framebuffer;
        u32 off_screen_mem_off;
        u16 off_screen_mem_size;
        u8  reserved1[206];
} VBEModeInfo;
