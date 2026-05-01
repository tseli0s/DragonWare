/**********************************************************************
 * FILE: node.h
 * PURPOSE: Device Manager node implementations and interfaces
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

#include "class.h"

#define MAX_DEVICE_NODE_NAME \
        (96) /* That is probably overkill, as we only need a generic name here. */

typedef enum _DeviceManagerNodePermissions {
        P_NONE          = 0x0,
        P_HAVE_CHILDREN = 0x1,
        P_MUTABLE       = 0x2,
        P_DIRECT_ACCESS = 0x4,
        P_USER          = 0x8
} DeviceManagerNodePermissions;

#define CheckNodeFlag(node, flag) (((node)->attr.permissions & (flag)) != 0)
#define SetNodeFlag(node, flag)   ((node)->attr.permissions |= (flag))
#define UnsetNodeFlag(node, flag) ((node)->attr.permissions &= ~(flag))
#define ResetNodeFlags(node)      ((node)->attr.permissions = P_NONE)
#define SuperNode                 (P_HAVE_CHILDREN | P_MUTABLE | P_DIRECT_ACCESS | P_USER)

/**
 * @brief A list of all possible device operations, with functions to actually commit them.
 * @warning Unused fields (That the device doesn't support) must not be set.*/
typedef struct _DeviceOperations {
        FramebufferDeviceOps framebuffer;
        ConsoleDeviceOps     console;
        UARTDeviceOps        uart;
} DeviceOperations;

/**
 * @brief Describes the purpose and capabilities of a single device.
 */
typedef struct _DeviceInterfaceTable {
        DeviceClass class; /** << Bitmap of the device classes, note that a device may do up to 64
                              different things at once. */
        DeviceOperations *ddo; /** << Device-dependent operations, see @ref DeviceClass*/
} DeviceInterfaceTable;

typedef struct _DeviceManagerNodeAttributes {
        char                         name[MAX_DEVICE_NODE_NAME];
        DeviceManagerNodePermissions permissions;
        Bool                         claimed;
        u64                          mmio_addr;
        u32                          mmio_len;
        u32                          kernel_mapped_addr;
} DeviceManagerNodeAttributes;

typedef struct _DeviceManagerNode {
        DeviceManagerNodeAttributes attr;
        DeviceInterfaceTable        devtable;
        void *private_state; /* Not used by the kernel directly. Drivers can put their own data
                                here, that they are responsible for freeing. */

        struct _DeviceManagerNode *child;
        struct _DeviceManagerNode *next;
} DeviceManagerNode;

/**
 * @brief Allocates a new device node (@ref DeviceManagerNode) and returns it, setting the internal
 * fields to the parameters given.
 * @param name The name of the node. Can be a duplicate.
 * @param permissions Permissions of the node (@ref DeviceManagerNodePermissions)
 * @param class Bitmap of the capabilities of this device, see @ref DeviceClass for more
 * @returns The newly allocated and initialized node on success, and a NullPointer on failure.
 */
DeviceManagerNode *MakeDeviceNode(const char *name, u32 permissions, DeviceClass class);

/**
 * @brief Rename a given node to the string given by @p new_name
 * @param node The node to rename
 * @param new_name The new name to give to the node
 * @note @p new_name must not exceed @ref MAX_DEVICE_NODE_NAME bytes
 */
[[gnu::nonnull(1, 2)]]
void RenameDeviceNode(DeviceManagerNode *node, const char *new_name);
