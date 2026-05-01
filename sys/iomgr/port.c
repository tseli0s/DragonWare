/**********************************************************************
 * FILE: port.c
 * PURPOSE: IPC port interface implementation
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "port.h"

#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <log.h>

typedef struct _PortNode {
        Port             *port;
        struct _PortNode *next;
} PortNode;

static PortNode *port_list_head = NullPointer;

Status InitializePortTree(void) {
        port_list_head = NullPointer;
        return STATUS_OK;
}

Status CreatePort(const char *name, Thread *owner, Port **portsave) {
        Bool is_global_port = true;
        if (!name) is_global_port = false;

        Port *port = kmalloc(sizeof(Port));
        if (!port) return STATUS_OUT_OF_MEMORY;

        if (name) {
                if (strlen(name) >= MAX_PORT_NAME) return STATUS_BAD_ARGUMENT;
                strncpy(port->name, name, MAX_PORT_NAME);
                port->name[MAX_PORT_NAME - 1] = '\0';
        }

        port->queue = NullPointer;
        port->owner = owner;

        if (is_global_port) {
                PortNode *node = kmalloc(sizeof(PortNode));
                if (!node) {
                        kfree(port->queue);
                        kfree(port);
                        return STATUS_OUT_OF_MEMORY;
                }

                node->port     = port;
                node->next     = port_list_head;
                port_list_head = node;
                LogMessage(LOG_INFO, "New global endpoint port: %s owned by %p", name, owner);
        }

        *portsave = port;
        return STATUS_OK;
}

Port *FindPortByName(const char *name) {
        PortNode *current = port_list_head;
        while (current) {
                if (strncmp(current->port->name, name, MAX_PORT_NAME) == 0) return current->port;
                current = current->next;
        }
        return NullPointer;
}

void DeletePort(Port *p) {
        PortNode *iter = port_list_head;
        PortNode *prev = NullPointer;

        while (iter) {
                if (iter->port == p) {
                        if (prev)
                                prev->next = iter->next;
                        else
                                port_list_head = iter->next;

                        kfree(iter);
                        kfree(p->queue);
                        kfree(p);
                        return;
                }
                prev = iter;
                iter = iter->next;
        }
}
