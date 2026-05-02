/**********************************************************************
 * FILE: object.h
 * PURPOSE: Kernel object definitions and exports
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

#pragma once

/* Because a bitfield is used to find free object slots in the process table, we can only parse the
 * native register bit size for every integer. cmake/CompilerOverrides.cmake defines PROCESSOR_BITS
 * to the amount of bits per general purpose register, usually 32. This can be patched to hold 64
 * bits in the future, but it would make parsing twice as expensive of course. */
#define MAX_OBJ_PER_PROCESS (PROCESSOR_BITS)

#include <ktypes.h>

/**
 * @brief A descriptor for binding an IRQ line to a port (Therefore configuring the kernel to
 * dispatch the IRQ to the port when it happens)
 */
typedef struct [[gnu::packed]] {
        u32 irq_no;   /** << IRQ line that will be bound to the port */
        u32 reserved; /** << Reserved for future expansion */
} IRQBindingDescriptor;

/**
 * @brief The type of an @ref Object object that its contents may be accessed under.
 * @sa Object
 */
typedef enum _ObjectType : unsigned long {
        OBJ_UNKNOWN = 0, /* Safety reasons in case an invalid zeroed out object is used */
        OBJ_DEVICE,
        OBJ_PORT,
        OBJ_SECTION,
} ObjectType;

/** @brief An operation to be performed on a device object */
typedef enum _DeviceObjectOp : unsigned long {
        DEVICE_GET,
        DEVICE_CLAIM,
        DEVICE_MAP,
} DeviceObjectOp;

/** @brief An operation to be performed on a port object */
typedef enum _PortObjectOp : unsigned long {
        PORT_CREATE,   /** << Create a new port */
        PORT_OPEN,     /** << Open an existing global port */
        PORT_WAIT_AT,  /** << Wait for messages to arrive to this port  */
        PORT_BIND_IRQ, /** << Wait for hardware events (IRQs) at this port */
        PORT_ACK_IRQ,  /** << Acknowledge an IRQ that was previously fired */
} PortObjectOp;

/** @brief An operation to be performed on a section object */
typedef enum _SectionObjectOp : unsigned long {
        SECTION_REQUEST, /** << Request the creation of a new section */
        SECTION_MAP,     /** << Map the section in the address space */
        SECTION_SHARE,   /** << Share the section's memory with another process */
} SectionObjectOp;

/**
 * @brief A kernel-allocated object used by processes to interact with the kernel API.
 * @details An object represents a generic @b kernel resource (e.g., device, port, memory section)
 * accessed through process handles.
 */
typedef struct _Object {
        ObjectType  type;
        const char *name;
        void       *data;
        u32         permissions;
        u32         refcnt;
} Object;

/**
 * @brief Used by @ref Process to hold handles to
 * kernel-allocated objects.
 */
typedef struct _HandleTable {
        Object *objlist[MAX_OBJ_PER_PROCESS]; /* Pointers save a lot of memory here */
        u32     valid_bitmap;                 /* Bitmap to find free slots between occupied ones. */
} HandleTable;

/* This is very important. Because the kernel uses a slab allocator, we want to fit the whole PCB in
 * a single 512 byte slab. The table already takes the majority of memory in a single PCB, so try to
 * keep it compact and fast in the expense of flexibility and limiting the amount of objects a
 * process can track. */
static_assert(sizeof(HandleTable) <= 256);

/**
 * @brief Allocates a single object and returns it.
 * @param[in] name Name of the object. If NullPointer, the object is anonymous and may not be
 * shared.
 * @param[in] type Type of the object allocated. See @ref ObjectType
 * @param[in] permissions Permissions of this object.
 * @note The actual Object::data field is NOT set. The caller is responsible for assigning
 * the object.
 * @return The object allocated on success or NullPointer if the allocation failed.
 */
Object *AllocateObject(const char *name, ObjectType type, u32 permissions);

/**
 * @brief Deletes an objects and frees up all the memory used by it.
 * @warning The internal Object::data field is not freed, the caller must free it beforehand.
 * @param[in] obj The object to delete
 */
[[gnu::nonnull]]
void DeleteObject(Object *obj);

/**
 * @brief Appends a single object to the @p table given.
 * @param[in,out] table The table to append to.
 * @param[in] obj The object to append.
 * @returns The index of the newly appended object, or -1 if there are no free slots.
 */
[[gnu::nonnull]]
int AppendToHandleTable(HandleTable *table, Object *obj);

/**
 * @brief Removes a single object to the @p table given.
 * @param[in,out] table The table to remove from.
 * @param[in] index Index to remove
 * @returns The object that was stored in that index or NullPointer if that @p index slot was not
 * occupied by an object.
 */
[[gnu::nonnull]]
Object *DeleteFromHandleTable(HandleTable *table, int index);
