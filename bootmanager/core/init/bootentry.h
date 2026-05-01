/**********************************************************************
 * FILE: bootentry.h
 * PURPOSE: Boot entry support abstraction and helpers
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#define MAX_BOOT_ENTRIES (6)

typedef struct _BootEntry {
        const char *name;
        void        (*SelectCallback)(void);
} BootEntry;

/**
 * @brief Add a new boot entry to be drawn and presented to the user.
 * @param[in] name
 * @param[in] index
 * @param[in] SelectCallback
 * @returns @ref STATUS_OK if the boot entry was registered succesfully. @ref STATUS_BAD_ARGUMENT if
 * an argument is @ref NullPointer or invalid. @ref STATUS_OUT_OF_BOUNDS if the index is larger than
 * the maximum amount of entries allowed.
 */
Status AddEntry(const char *name, unsigned int index, void (*SelectCallback)(void));

/**
 * @brief Update the currently selected entry to be @p new.
 * @param[in] new Index of the boot entry selected. Must be >= 0.
 */
void UpdateIndex(unsigned int new);

/**
 * @brief Get the currently selected entry.
 */
Size GetCurrentIndex(void);

/**
 * @brief Execute the callback related to a boot entry.
 * @warning This function may not return.
 */
void EntrySelected();

/**
 * @brief Get the array containing all the boot entries held by the bootloader.
 * @param[out] n_entries Pointer to where to store the amount of entries registered
 * @returns A pointer to the start of the array containing all the boot entries.
 */
BootEntry *GetAllBootEntries(Size *n_entries);
