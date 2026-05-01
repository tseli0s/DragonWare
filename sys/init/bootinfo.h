/**********************************************************************
 * FILE: bootinfo.h
 * PURPOSE: Access to multiboot information structures
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <macros.h>

#include "vendor/multiboot.h"

/**
 * @brief Copy the multiboot information struct, given to the kernel upon handing off from the
 * bootloader, into kernel-reserved memory.
 * @param bootinfo The Multiboot structure to copy. Cannot be a NullPointer.
 */
[[gnu::nonnull(1)]]
void RegisterBootInformation(Multiboot *bootinfo);

/**
 * @brief Registers the amount of boot modules and their addresses into kernel internal structures
 * for fast access to them. Called early at boot
 */
void RegisterBootModules(void);

/**
 * @brief Returns the Multiboot specification structure given to the kernel at boot. The structure
 * is copied early at boot in kernel memory so it is safe to access this at any point.
 */
Multiboot *GetBootInformationStructure(void);

/**
 * @brief Fetch the boot module information (Based on the multiboot protocol) and place it at @p
 * dest.
 * @param index Index on the array of modules provided by the bootloader.
 * @param dest Where to write the data. Must not be a NullPointer.
 * @return STATUS_OK if the module was found and registered, STATUS_OUT_OF_BOUNDS if @p index is
 * larger than the amount of modules loaded.
 */
[[gnu::nonnull(2)]]
Status GetBootModuleAt(unsigned int index, MultibootModule *dest);

/**
 * @brief Checks whether the bootloader provided a specific command line flag to the kernel. Each
 * flag is separated by spaces.
 * @param opt The option to check for.
 * @return Whether the option was provided in the command line or not.
 */
Bool BootOptionProvided(const char *opt);
