/**********************************************************************
 * FILE: port.h
 * PURPOSE: IPC port interface exports
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <spinlock.h>

#include "task/task.h"

#define MAX_PORT_NAME        (48)
#define MAX_MESSAGES_IN_PORT (12)

/**
 * @brief A single IPC endpoint to send messages to.
 * @details A port is an endpoint of communication for IPC messages. In DragonWare, they are used
 * for service resolving (Which part of the OS handles which service?), secure communication and
 * fast data exchange between two processes.
 */
typedef struct _Port {
        Message  msgbuf[MAX_MESSAGES_IN_PORT]; /* Messages held in this port */
        Size     head, tail, count; /* Indices of the messages currently in the message buffer */
        char     name[MAX_PORT_NAME];
        Spinlock lock;  /* Placeholder for the future, not gonna be used now */
        Thread  *owner; /* Owner of this port. Messages go here. */
} Port;

/**
 * @brief Initializes the tree holding IPC ports in the kernel.
 * @return STATUS_OK if the tree was initialized successfully and is ready to be used.
 */
Status InitializePortTree(void);

/**
 * @brief Create a single port and add it to the global list of ports.
 * @sa Port
 * @param[in] name Name of the port. If NullPointer, this cannot be shared globally.
 * @param[in] owner Thread that will receive messages from this port.
 * @param[out] save_port Where to place the allocated @ref Port, must not be a NullPointer.
 * @return STATUS_OK if the port was allocated succesfully, STATUS_OUT_OF_MEMORY if @ref kmalloc
 * failed.
 */
[[gnu::nonnull(2, 3)]]
Status CreatePort(const char *name, Thread *owner, Port **save_port);

/**
 * @brief Find a port in the port list inside the kernel and return a pointer to its object.
 * @param[in] name The name of the port.
 * @return The port if found, or NullPointer if the port does not exist in the tree.
 */
[[gnu::nonnull]]
Port *FindPortByName(const char *name);

/**
 * @brief Deletes a port and frees up all the memory used by it.
 * @param[in] p The port to delete, must not be NullPointer.
 */
[[gnu::nonnull]]
void DeletePort(Port *p);
