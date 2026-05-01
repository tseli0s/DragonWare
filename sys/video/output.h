/**********************************************************************
 * FILE: output.h
 * PURPOSE: Kernel-mode output access for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "iomgr/node.h"

typedef struct _OutputNode {
        DeviceManagerNode  *node;
        struct _OutputNode *next;
} OutputNode;

#define ForEachConsoleDevice(__LAMBDA__)                                           \
        do {                                                                       \
                for (OutputNode *curr = GetAllOutputsKnown(); curr != NullPointer; \
                     curr             = curr->next) {                                          \
                        if (SupportsClass(curr->node, DEVCLASS_CONSOLE)) {         \
                                __LAMBDA__                                         \
                        }                                                          \
                }                                                                  \
        } while (0);

/**
 * @brief Fetches a list of all outputs that the kernel currently knows of.
 * @note An output is anything that the kernel can produce a visible change on.
 * @returns The list of all outputs (May be a @ref NullPointer)
 */
OutputNode *GetAllOutputsKnown(void);

/**
 * @brief Adds a new output device
 * @warning Don't use this directly. Instead let the device manager call it when appropriate.
 * @param dev The device to register. Must not be a NullPointer.
 */
[[gnu::nonnull(1)]]
void AddNewOutputDevice(DeviceManagerNode *dev);

/**
 * @brief Selects a given @p node as the primary output device
 * @param node The node to use for primary kernel output
 * @note @p node is allowed to be a @ref NullPointer, when the kernel does not need to output
 * anymore.
 */
[[gnu::nonnull(1)]]
void SelectPrimaryOutputDevice(OutputNode *node);

/**
 * @brief Removes an output device from being used by the kernel (eg. when a device is claimed)
 * @param node The device to remove, matched inside the internal @ref OutputNode list
 * @returns STATUS_OK on success, STATUS_NOT_FOUND If the node wasn't found
 */
[[gnu::nonnull(1)]]
Status RemoveKernelOutput(DeviceManagerNode *node);

/** @brief Returns the primary output device used by the kernel */
OutputNode *GetPrimaryOutputDevice(void);
