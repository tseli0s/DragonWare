/**********************************************************************
 * FILE: modeset.h
 * PURPOSE: Modesetting functions for the bootloader
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>

#include "proto/vbe.h"

/**
 * @brief Get the video card information for the VESA-compatible card used.
 */
extern VBEInfo GetVESACardInformation(void);

/**
 * @brief Get the video modes and other information for the video card used.
 * @warning I have no idea what happens if there are many graphics card on a system.
 * @note Like @ref ModesetInRealMode, this needs to temporarily drop back to real mode.
 */
extern VBEModeInfo GetVESAVideoInfo(void);

/**
 * @brief Sets up a preconfigured video mode to be used as the framebuffer
 * @param mode The mode to use. See @ref GetVESAVideoInfo
 * @note This will temporarily jump back to real mode
 */
extern void ModesetInRealMode(u32 mode);
