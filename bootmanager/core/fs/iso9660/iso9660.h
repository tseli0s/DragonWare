/**********************************************************************
 * FILE: iso9660.h
 * PURPOSE: ISO9660 filesystem exports
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "../fs.h"

/**
 * @brief Open a file object and place the relevant metadata in @p output
 * @param[in] p Partition/volume where the file is stored.
 * @param[in] path Path to the file.
 * @param[out] output Where to write the file metadata for further use, eg. with @ref
 * ReadFile_ISO9660.
 * @return STATUS_OK on success, another @ref Status value on failure.
 */
[[gnu::nonnull(2, 3)]]
Status OpenFile_ISO9660(const Partition p, const char *path, File *output);

/**
 * @brief Read from a file inside a ISO9660 formatted volume.
 * @param[in] f File object to read metadata from, see @ref OpenFile
 * @param[in] n_bytes Number of bytes to read, must be above 0.
 * @param[out] output Pointer to memory where the file contents will be placed
 * @return Amount of bytes read; If 0, failure can be assumed.
 */
int ReadFile_ISO9660(File *f, Size n_bytes, Byte *output);
