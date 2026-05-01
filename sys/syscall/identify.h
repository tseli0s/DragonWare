/**********************************************************************
 * FILE: identify.h
 * PURPOSE: DWSystemIdentify system call helpers
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#define SI_MAX_NAME (24)
#define SI_MAX_TAG  (12)

/**
 * @brief System information data returned by the kernel, that allow applications to access
 * information about the operating system's version and build specifications.
 */
typedef struct _SystemIdentify {
        char name[SI_MAX_NAME]; /** << Name of the operating system. This should be "DragonWare" for
                                 normal DragonWare builds.*/
        char tag[SI_MAX_TAG]; /** << Build tag. Defines for what purpose was the OS built (eg. -dev
                               for a development, unstable version) */
        u32  major;           /** << Major release number of DragonWare. */
        u32  minor;           /** << Minor release number of DragonWare. */
        u32  patch;           /** << Patch release number of DragonWare. */
        u64  build_id; /** << Build ID. Do not rely on this value, it is only intended for debugging
                          and reproducible builds. */
} SystemIdentify;
