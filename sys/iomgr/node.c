/**********************************************************************
 * FILE: node.c
 * PURPOSE: Device Manager node implementations and interfaces
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "node.h"

#include <kmalloc.h>
#include <kstring.h>

#include "class.h"
#include "devmgr.h"
#include "ktypes.h"
#include "log.h"

DeviceManagerNode *MakeDeviceNode(const char *name, u32 permissions, DeviceClass class) {
        if (!name || name[0] == '\0') return NullPointer;
        if (strlen(name) > MAX_DEVICE_NODE_NAME) return NullPointer;
        DeviceManagerNode *node = kzalloc(sizeof(DeviceManagerNode));
        if (!node) return NullPointer;

        strncpy(node->attr.name, name, MAX_DEVICE_NODE_NAME);
        node->attr.name[MAX_DEVICE_NODE_NAME - 1] = '\0'; /* Just to be sure */

        node->attr.claimed     = false;
        node->attr.permissions = permissions;
        node->next             = NullPointer;
        node->child            = NullPointer;

        /* devtable->ddo will be set by the driver manually */
        node->devtable = (DeviceInterfaceTable){
                .class = class,
                .ddo   = NullPointer,
        };
        node->private_state = NullPointer;

        return node;
}

void RenameDeviceNode(DeviceManagerNode *node, const char *new_name) {
        if (node == GetRootDeviceManagerNode())
                LogMessage(LOG_WARNING,
                           "Renaming root device tree node! This should be done carefully.");
        strncpy(node->attr.name, new_name, MAX_DEVICE_NODE_NAME);
        node->attr.name[MAX_DEVICE_NODE_NAME - 1] = '\0';
}
