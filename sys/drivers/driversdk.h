/**********************************************************************
 * FILE: driversdk.h
 * PURPOSE: Kernel driver support for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/**
 * @brief Maximum length for a driver name.
 * @details This limit includes the terminating null byte.
 */
#define MAX_DRIVER_NAME (32)

/**
 * @brief SDK header for writing drivers for DragonWare.
 * @details Provides standard types and macros for declaring driver descriptors and
 * exporting symbols.
 */

#ifndef __GNUC__
#warning Compiler is not GCC-like, attributes may not work!
#endif /* __GNUC__ */

/**
 * @brief Function pointer type for driver initialization.
 * @returns STATUS_OK on success, other values on failure.
 */
typedef Status (*InitCall)(void);

/**
 * @brief Driver descriptor structure.
 * @details Placed in the .drivers section via @ref ADD_DRIVER_DESCRIPTOR. Contains
 * basic metadata and entry points for driver initialization and teardown.
 */
typedef struct [[gnu::packed]] _DriverDescriptor {
        char name[MAX_DRIVER_NAME]; /**< Driver name. */
        Bool init_earlier; /**< If true, the kernel will try to initialize this driver before others
                            */
        InitCall __init;   /**< Initialization callback. */
} DriverDescriptor;

/**
 * @brief Register a driver descriptor in the .drivers section.
 */
#define ADD_DRIVER_DESCRIPTOR(x) \
        __attribute__((used, section(".drivers"))) const DriverDescriptor *__drv_##x = &x

/**
 * @brief Export a symbol with default visibility.
 * @details Equivalent to GCC/Clang's visibility attribute.
 */
#define EXPORT_SYMBOL(x) __attribute__((visibility("default"))) x
