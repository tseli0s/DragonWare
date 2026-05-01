/**********************************************************************
 * FILE: fs.h
 * PURPOSE: Lightweight filesystem abstraction
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "storage/partition.h"

/**
 * @brief A struct describing a single file opened from a system volume.
 * @sa OpenFile
 */
typedef struct _File {
        void *internal; /** Filesystem drivers set this */
        Size  cursor;   /** < Where did the reading last stop. Next call to @ref ReadFromFile
                         *   will return data starting from this offset inside the file */
        Size  filesize; /** < Size of the file in bytes.  */
        Bool loaded; /** < If true, the file has been succesfully loaded into memory and may be read
                      */
        Bool ended;  /** < If true, cursor can't advance and therefore we've reached EOF */
        PartitionType t; /** < Used to figure out which filesystem driver will perform the read */
} File;

/**
 * @brief Opens a file and returns a @ref File that can be used to further access the file's
 * contents.
 * @param[in] volume The volume name where the file is stored. For example hd0p0 for the first
 * partition of the first hard disk.
 * @param[in] path The path to the file within that volume. Leading slash must be omitted.
 * @warning Some filesystem drivers don't implement subdirectory searching, meaning the files might
 * have to be stored in the root directory to be found.
 * @returns a @ref File, where the loaded field, if set to true, indicates success, and false
 * indicates that the file couldn't be opened; See debug logs for details.
 */
[[gnu::nonnull(1, 2)]]
File OpenFile(const char *volume, const char *path);

/**
 * @brief Reads @p n_bytes from @ref File @p f into @p output.
 * @param[in] f Pointer to the file that was opened before. See @ref OpenFile.
 * @param[out] output Where to copy the file's data. Must not be NullPointer.
 * @param[in] n_bytes Amount of bytes to read. The bytes that will be actually read may be less.
 * @returns The amount of bytes read. There's no guarantee that n_bytes will match this number.
 */
[[gnu::nonnull(1, 2)]]
Size ReadFromFile(File *f, Byte *output, Size n_bytes);

/**
 * @brief Frees up any resources used by a file
 * @param[in] f The file where the internal driver will perform any cleanups.
 * @note @p f will no longer be valid after this call.
 */
[[gnu::nonnull(1)]]
void CloseFile(File *f);
