/**********************************************************************
 * FILE: elfloader.h
 * PURPOSE: Minimal and fast ELF loading implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "fs/fs.h"

/**
 * @brief Reads an opened ELF file directly into high memory,
 * @warning The file's cursor will be reset before reading this.
 * @param[in] file The file to read
 * @param[out] entry The entry point in that file, where the bootloader must jump to.
 * @return STATUS_OK on success, other @ref Status values on failure.
 */
Status ReadELFToMemory(File *file, uintptr_t *entry);
