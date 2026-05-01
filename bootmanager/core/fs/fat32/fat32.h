/**********************************************************************
 * FILE: fat32.h
 * PURPOSE: FAT32 support and implementation (Without LFN)
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "../fs.h"
#include "storage/partition.h"

/* Opens a file using the FAT32 driver */
Status OpenFile_FAT32(const Partition p, const char *path, File *output);

int ReadFile_FAT32(File *f, Size n_bytes, void *output);
