/**********************************************************************
 * FILE: devmgr.h
 * PURPOSE: DragonWare Device Manager constants, helpers and definitions
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

#include "node.h"

/**
 * @brief Run a specific code block for every @ref DeviceManagerNode sibling of @p _root
 * @sa ForEachChildDevice
 * @note The current device in the walk operation is named curr and may be accessed using that
 * identifier
 */
#define ForEachSiblingDevice(_root, __LAMBDA__)                                                 \
        do {                                                                                    \
                for (DeviceManagerNode *curr = _root; curr != NullPointer; curr = curr->next) { \
                        __LAMBDA__                                                              \
                }                                                                               \
                                                                                                \
        } while (0);

/**
 * @brief Run a specific code block for every @ref DeviceManagerNode child of @p _root
 * @sa ForEachSiblingDevice
 * @note The current device in the walk operation is named curr and may be accessed using that
 * identifier
 */
#define ForEachChildDevice(_root, __LAMBDA__)                                     \
        do {                                                                      \
                for (DeviceManagerNode *curr = _root->child; curr != NullPointer; \
                     curr                    = curr->next) {                                         \
                        __LAMBDA__                                                \
                }                                                                 \
                                                                                  \
        } while (0);

/** @brief Initializes the device manager subsystem for use. */
Status InitDeviceManager(void);

/**
 * @brief Fetches the @ref DeviceManagerNode at the given path and returns it.
 * @param path The path to the node, eg. "/Devices/CPU/CPU0".
 * @note The path must be separated by slashes (/). Every slash means child of the token before.
 * @returns The device node if the device is found, or NullPointer if the device node isn't found.
 */
[[gnu::nonnull(1)]]
DeviceManagerNode *GetDeviceFromPath(const char *path);

/**
 * @brief Adds a new device to the device manager.
 * @note This should usually only be called from drivers, and not directly by the kernel.
 * @param parent The parent of the new node. If NullPointer, it will be added to the root.
 * @param new_node The new node to add. Must never be NullPointer.
 */
[[gnu::nonnull(2)]]
void AddDevice(DeviceManagerNode *parent, DeviceManagerNode *new_node);

/**
 * @brief Removes a device from the device manager tree.
 * @param device The device to remove (may be fetched using @ref GetDeviceFromPath). Must not be a
 * NullPointer.
 */
Status RemoveDevice(DeviceManagerNode *device);

/** @brief Returns the root device manager node ("/"). */
DeviceManagerNode *GetRootDeviceManagerNode(void);
