/**********************************************************************
 * FILE: object.h
 * PURPOSE: Kernel object wrappers for user libraries
 * PROJECT: DragonWare User Library
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "kernelapi.h"
#include "kerneltypes.h"

/**
 * @brief A handle to a kernel object used to reference kernel objects when creating or invoking
 * them.
 * @sa CreateObject
 */
typedef int Handle;

/**
 * @brief Opcode integer to be used when invoking objects, see @ref PortObjectOp for an example of
 * port objects and their possible operations.
 * @sa InvokeObject
 */
typedef unsigned long ObjectOperation;

/**
 * @brief Allocate a new kernel object and return a handle to it.
 * @param[in] name The name of the object. If NullPointer, this object cannot be shared with other
 * processes.
 * @param type Type of the object. See @ref ObjectType for more details.
 * @param permissions Permissions of the object. Unused by the kernel.
 * @returns The @ref Handle used to identify the created object, or a negative integer on failure.
 */
Handle CreateObject(const char *name, ObjectType type, u32 permissions);

/**
 * @brief Invokes the operation of an object according to its type.
 * @param[in] handle Handle to the object that will be invoked.
 * @param op Object operation to be invoked. See documentation of @ref ObjectOperation
 * @param[in,out] args Extra arguments that may be passed to or received from the kernel.
 * @returns The status of the operation (STATUS_OK is the success value, other values depend on the
 * object, the operation invoked and the exact documentation of them)
 */
Status InvokeObject(Handle handle, ObjectOperation op, void *args);

/**
 * @brief Deletes an object and frees up all the memory used by it.
 * @param[in] handle Handle to the object that is to be deleted. Must be returned by @ref
 * CreateObject.
 */
void DeleteObject(Handle handle);
