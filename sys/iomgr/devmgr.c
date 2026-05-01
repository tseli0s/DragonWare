/**********************************************************************
 * FILE: devmgr.c
 * PURPOSE: DragonWare Device Manager implementation
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "devmgr.h"

#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "class.h"
#include "node.h"
#include "video/output.h"

#define ROOT_DEVICE_NAME "Devices"

static DeviceManagerNode *root = NullPointer;

/* Recursive version for the function after */
static DeviceManagerNode *_FindDeviceParent(DeviceManagerNode *dev, DeviceManagerNode *node) {
        if (!node || !dev) return NullPointer;

        ForEachChildDevice(node, {
                if (curr->child == dev) return node;

                DeviceManagerNode *p = _FindDeviceParent(dev, curr);
                if (p) return p;
        });

        return NullPointer;
}

/**
 * @brief Starting at root, traverse the entire tree and look for a device node with that name.
 * @p dev The device node to search the parent for
 * @returns The parent node on success, or @ref NullPointer if it could not be found.
 */
static DeviceManagerNode *FindDeviceParentOf(DeviceManagerNode *dev) {
        return _FindDeviceParent(dev, root);
}

Status InitDeviceManager(void) {
        /* It's already present. Nothing bad happened, so just act like this check
         * doesn't even exist. */
        if (root) return STATUS_OK;

        root = kzalloc(sizeof(DeviceManagerNode));
        if (!root) return STATUS_OUT_OF_MEMORY;

        strncpy(root->attr.name, ROOT_DEVICE_NAME, MAX_DEVICE_NODE_NAME);
        root->attr.name[MAX_DEVICE_NODE_NAME - 1] = '\0';
        root->attr.claimed                        = false;
        root->attr.permissions                    = P_MUTABLE | P_HAVE_CHILDREN;
        root->next                                = NullPointer;
        root->child                               = NullPointer;

        LogMessage(LOG_INFO, "Device Manager brought online.");
        return STATUS_OK;
}

DeviceManagerNode *GetDeviceFromPath(const char *path) {
        /* Paths must start with a slash, relative paths aren't supported and won't be for a while.
         * And obviously a zero-length string makes no sense, we assume bad memory was passed as a
         * string.
         */
        if (*path == '\0' || path[0] != '/') return NullPointer;

        DeviceManagerNode *node  = root;
        const char        *start = path + 1;
        char               token[MAX_DEVICE_NODE_NAME + 4];

        const char *next_slash = strchr(start, '/');
        size_t      first_len  = next_slash ? (size_t)(next_slash - start) : strlen(start);

        /* Skip the first node which is the root node */
        if (first_len > 0 && strncmp(start, node->attr.name, first_len) == 0 &&
            node->attr.name[first_len] == '\0')
                start = next_slash ? next_slash + 1 : start + first_len;

        while (*start) {
                while (*start == '/') start++;
                if (*start == '\0') break;

                const char *end = start;
                while (*end && *end != '/') end++;

                size_t len = (uintptr_t)end - (uintptr_t)start;
                /* Huge token, don't even try to compare it */
                if (len >= sizeof(token)) return NullPointer;

                memcpy(token, start, len);
                token[len] = '\0';

                DeviceManagerNode *child = node->child;
                while (child) {
                        if (strcmp(child->attr.name, token) == 0) break;
                        child = child->next;
                }

                /* No more children and we didn't find anything */
                if (!child) return NullPointer;

                node  = child;
                start = end;
        }

        return node;
}

void AddDevice(DeviceManagerNode *parent, DeviceManagerNode *new_node) {
        DeviceManagerNode *actual_parent = parent ? parent : GetRootDeviceManagerNode();
        if (!actual_parent) return;

        if (!CheckNodeFlag(actual_parent, P_HAVE_CHILDREN) ||
            !CheckNodeFlag(actual_parent, P_MUTABLE)) {
                LogMessage(LOG_ERROR, "Node %p cannot have children or be mutated!", actual_parent);
                return;
        }
        new_node->next = NullPointer;

        LogMessage(LOG_DEBUG, "Adding device node %p (name: %s) to %p (%s)", new_node,
                   new_node->attr.name, actual_parent, actual_parent->attr.name);

        if (SupportsClass(new_node, DEVCLASS_FRAMEBUFFER) ||
            SupportsClass(new_node, DEVCLASS_CONSOLE)) {
                LogMessage(LOG_DEBUG, "New output device: %s at %p", new_node->attr.name, new_node);
                AddNewOutputDevice(new_node);
        }
        if (!actual_parent->child) {
                actual_parent->child = new_node;
                return;
        }

        DeviceManagerNode *iter = actual_parent->child;
        while (iter->next) iter = iter->next;
        iter->next = new_node;
}

Status RemoveDevice(DeviceManagerNode *device) {
        if (!device || device == root) return STATUS_BAD_ARGUMENT;

        DeviceManagerNode *c = device->child;
        while (c) {
                DeviceManagerNode *next = c->next;
                RemoveDevice(c);
                c = next;
        }

        DeviceManagerNode *parent = FindDeviceParentOf(device);
        if (parent) {
                DeviceManagerNode **pp = &parent->child;
                while (*pp && *pp != device) pp = &(*pp)->next;
                if (*pp == device) *pp = device->next;
        }

        kzeromem(device, sizeof(*device));
        kfree(device);
        return STATUS_OK;
}

DeviceManagerNode *GetRootDeviceManagerNode(void) { return root; }
