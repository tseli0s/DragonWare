/**********************************************************************
 * FILE: vbe.h
 * PURPOSE: VESA BIOS Extensions standard definitions
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#define VBE_FLAG_SWITCHABLE_DAC    (0x1) /* Whatever the fuck that means */
#define VBE_ADAPTER_VGA_COMPATIBLE (0x2)
#define VBE_RAMDAC_FIX             (0x4) /* Literally what the hell is that */

#define VESA_LINEAR_FB             (0x4000)
#define VESA_PRESERVE_MEM          (0x8000)

#define VBE_GET_BIOS_INFO          (0x4F00)
#define VBE_GET_MODE_INFO          (0x4F01)
#define VBE_SET_VIDEO_MODE         (0x4F02)
#define VBE_SUCCESS                (0x004F)

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
        Byte  reserved[222];
        char  oem_data[256];
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

/**
 * @brief Returns the video card's VESA information block.
 * @param[out] info Pointer to the allocated @ref VBEInfo structure to write the contents to.
 * @warning @p info must be within the first megabyte of physical memory.
 * @return STATUS_OK on success, STATUS_BAD on failure. May invoke @ref FatalError
 */
[[gnu::nonnull]]
Status GetVESAInformationBlock(VBEInfo *info);

/**
 * @brief Finds the best matching VESA mode compared to the width, height and depth requested.
 * @todo I should probably have a better algorithm to decide the best mode and prune the rest if no
 * modes match perfectly... Anyways
 * @param[in] info VESA BIOS information block, must not be a @ref NullPointer. See @ref
 * GetVESAInformationBlock
 * @param[out] modeinfo VESA video mode information. When the mode is selected, information about
 * the mode will be written in that pointer.
 * @param w Desired width
 * @param h Desired height
 * @param d Desired depth (Bits per pixel)
 * @param[out] mode Where to return the mode number. Must not be a @ref NullPointer.
 * @returns STATUS_OK if the modes match. STATUS_NOT_FOUND if another similar mode was found but not
 * an exact match. STATUS_BAD if an error occured.
 */
[[gnu::nonnull]]
Status FindBestVESAMode(VBEInfo *info, VBEModeInfo *modeinfo, int w, int h, int d, u16 *mode);

/**
 * @brief Switch to the VESA mode given by number @p mode
 * @param mode Mode number to use.
 * @warning In @p mode the bits 14 and 15 must not be specified manually. The framebuffer will
 * always be of linear form and the screen will always be cleared upon modesetting.
 * @return STATUS_OK if the mode was loaded succesfully, STATUS_BAD if the call to the BIOS failed
 * or returned error values.
 */
Status VESAModeset(u16 mode);
