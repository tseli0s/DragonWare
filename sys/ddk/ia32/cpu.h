/**********************************************************************
 * FILE: cpu.h
 * PURPOSE: CPU-specific definitions and exports
 * PROJECT: DragonWare Kernel
 * DATE: 09-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "kcpuid.h"

/**
 * @brief Initializes architecture-specific parts of the CPU.
 * @returns STATUS_OK on success, STATUS_BAD on failure
 */
Status ArchInit(void);

/**
 * @brief Checks if a given feature is supported in the processor or not
 * @param feat The feature to check for, must be a member of @ref x86Features
 * @note Requires CPUID support to be present; If missing, manual probing may be required.
 * @returns true if the feature is supported, false otherwise.
 */
Bool x86FeatureSupported(x86Features feat);
