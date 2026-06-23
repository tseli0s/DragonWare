/**********************************************************************
 * FILE: spleen.h
 * PURPOSE: Reexport of Spleen font used by kernel for framebuffer text drawing
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifdef __DRAGONWARE_SYS__ /* Only defined for the kernel build, not for the userland */
#include <ktypes.h>
#else
#include <kerneltypes.h>
#endif /* __DRAGONWARE_SYS__ */

/*
 * Pointer to the font that the kernel uses.
 * The font is placed in a special section called .fontdata,
 * so that we can map it separately later.
 */
[[gnu::section(".fontdata"), gnu::visibility("default")]]
extern const Byte _kernel_font[];
