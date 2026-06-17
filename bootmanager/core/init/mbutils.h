/**********************************************************************
 * FILE: mbutils.h
 * PURPOSE: Multiboot protocol utilities
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/**
 * @brief Returns the offset within @p multiboot_file that a Multiboot header is found
 * @param multiboot_file A file read into memory that will be searched for the multiboot header.
 * @return The offset within @p multiboot_file if found (starting from 0), -1 otherwise.
 */
[[gnu::nonnull(1)]]
off_t FindMultibootHeader(u8 *multiboot_file);
