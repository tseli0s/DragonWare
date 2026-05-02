/**********************************************************************
 * FILE: kernelapi.h
 * PURPOSE: Microkernel API system call exports for userspace applications
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef _KERNEL_API_H
#define _KERNEL_API_H           1

#define SYSCALL_IDENTIFY        (0)
#define SYSCALL_EXIT            (1)
#define SYSCALL_YIELD           (2)
#define SYSCALL_KLOG            (3)
#define SYSCALL_RAISE_IOPL      (4)
#define SYSCALL_SEND            (5)
#define SYSCALL_RECEIVE         (6)
#define SYSCALL_TICK_SINCE_BOOT (7)
#define SYSCALL_CREATE_OBJECT   (8)
#define SYSCALL_INVOKE_OBJECT   (9)
#define SYSCALL_DELETE_OBJECT   (10)

#include "cabi.h"
#include "cppsupport.h"
#include "ipc86.h"
#include "kerneltypes.h"

#define SI_MAX_NAME (24)
#define SI_MAX_TAG  (12)

DW_BEGIN_DECLS

/**
 * @brief System information data returned by the kernel, that allow applications to access
 * information about the operating system's version and build specifications.
 */
typedef struct _SystemIdentify {
        char name[SI_MAX_NAME]; /** << Name of the operating system. This should be "DragonWare" for
                                 normal DragonWare builds.*/
        char tag[SI_MAX_TAG]; /** << Build tag. Defines for what purpose was the OS built (eg. -dev
                               for a development, unstable version) */
        u32  major;           /** << Major release number of DragonWare. */
        u32  minor;           /** << Minor release number of DragonWare. */
        u32  patch;           /** << Patch release number of DragonWare. */
        u64  build_id; /** << Build ID. Do not rely on this value, it is only intended for debugging
                          and reproducible builds. */
} SystemIdentify;

/**
 * @brief Device descriptor for a claimed device returned by @ref _DWDeviceClaim.
 */
typedef struct [[gnu::packed]] _DeviceMapDescriptor {
        u32 irq;       /** < IRQ occupied by this device. If 0, this device does not use IRQs. */
        u64 mmio_addr; /** < Address where this device is mapped. If 0, this device cannot be
                          accessed through MMIO. */
        u64 mmio_len;  /** < Length of the physical address of this device's mapping. Only valid if
                          mmio_addr is also valid. */
} DeviceMapDescriptor;

/**
 * @brief A descriptor for binding an IRQ line to a port (Therefore configuring the kernel to
 * dispatch the IRQ to the port when it happens)
 * @sa PortObjectOp
 */
typedef struct [[gnu::packed]] {
        u32 irq_no;   /** << IRQ line that will be bound to the port */
        u32 reserved; /** << Reserved for future expansion */
} IRQBindingDescriptor;

/** @brief Flags describing the permissions of a single section. */
typedef enum _SectionPermissions : unsigned long {
        SECTION_NONE      = 0x00, /** << Nothing allowed on */
        SECTION_WRITEABLE = 0x01, /** << Section can have its memory written. */
        SECTION_SHAREABLE = 0x02, /** << Section may be shared with another process */
        SECTION_CACHEABLE = 0x04, /** << Page writes may be cached by hardware and may not take
                                     effect immediately. */
} SectionPermissions;

/** @brief A descriptor for a section request passed from user programs to the kernel. */
typedef struct [[gnu::packed]] _UserSectionDescriptor {
        Size needed_pages; /** << Amount of needed pages. Must be above zero and less than @ref
                              MAX_SECTION_FRAMES */
        SectionPermissions perms; /** << Permissions bitfield. See @ref SectionPermissions */
} UserSectionDescriptor;

/**
 * @brief The type of an object that its contents may be accessed under in the kernel when
 * operations are performed on an object.
 */
typedef enum _ObjectType : unsigned long {
        OBJ_UNKNOWN = 0, /* Safety reasons in case an invalid zeroed out object is used */
        OBJ_DEVICE,      /** << Device manager object */
        OBJ_PORT,        /** << Port (IPC endpoint) object */
        OBJ_SECTION,     /** << Section (Memory management) object */
} ObjectType;

/**
 * @brief An operation to be performed on a device object
 * @warning Device management is a privileged operation in DragonWare - Only processes directly
 * loaded by the kernel can invoke such operations.
 */
typedef enum _DeviceObjectOp : unsigned long {
        DEVICE_GET,   /** Get the device given, and hold it in the handle */
        DEVICE_CLAIM, /** Claim the device */
        DEVICE_MAP,   /** Map the device's MMIO range to the given base address */
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
 * @brief _DWSystemIdentify system call (#0) wrapper
 * @details This function provides the caller with build and version information about DragonWare's
 * underlying information at runtime. The data is copied inside @p saveptr, which must be a locally
 * allocated pointer (ie. Not returned by the kernel).
 * @param[out] saveptr A pointer to a @ref SystemIdentify allocated memory block that must be
 * accessible by the caller, where the system information will be saved.
 * @sa SystemIdentify
 */
[[gnu::nonnull]]
void _cdecl _DWSystemIdentify(SystemIdentify *saveptr);

/**
 * @brief _DWExit system call (#1) wrapper
 * @details This function will immediately terminate the calling process and clean up all the memory
 * used by it. Pending IPC messages will be ignored. This function should be called, usually, once
 * the program has finished its tasks and has nothing left to do.
 * @returns Never, as the kernel will immediately terminate the calling process.
 */
void _cdecl noreturn _DWExit(void);

/**
 * @brief _DWYield system call (#2) wrapper.
 * @details This function will drop the remaining CPU time slice of the caller process and let the
 * kernel pick a new process to receive process time. This may be useful, eg. If the process is
 * waiting on a message but does nothing in the meantime, and therefore, other processes could use
 * its time.
 * @note When trapping into the @ref _DWIPCReceive system call, this function is unnecessary; The
 * kernel is going to preemptively block the process and let a new one run.
 */
void _cdecl _DWYield(void);

/**
 * @brief _DWklog system call (#3) wrapper.
 * @details This will copy @p msg into the kernel log buffer, classifying it by @p level. Only
 * processes with the C_PROC_KLOG capability are allowed to use this system call. Processes not
 * assigned this capability can consider this system call a NOOP one.
 * @param level Severity of the log message, see @ref LogLevel for more details.
 * @param msg The message to write to the kernel logs. Must not be NULL.
 * @sa LogLevel
 */
[[gnu::nonnull]]
void _cdecl _DWklog(LogLevel level, const char *msg);

/**
 * @brief _DWRaiseIOPL system call (#4) wrapper.
 * @details This function will attempt to set bits 12-13 in the EFLAGS register of the calling
 * process allowing it to manage hardware directly by talking to I/O ports. Only processes that the
 * kernel trusts (Usually its own userspace drivers only) can succeed in this call, see the warning
 * below as to why.
 * @warning This is a very dangerous function - It is only intended for DragonWare microkernel
 * drivers, which is why there's an entire separate capability just for this function. Processes
 * having the relevant permission set are able to adjust timer frequency, read directly from the
 * disk (bypassing any drivers and permissions) and access user data without any checks.
 * @return STATUS_OK if the process has the capability to talk to I/O ports directly.
 * STATUS_UNSUPPORTED if the process is not allowed to talk to I/O ports because it lacks the
 * kernel-assigned capability.
 */
Status _cdecl _DWRaiseIOPL(void);

/**
 * @brief _DWIPCSend system call (#5) wrapper
 * @details This routine is going to store a message and wake the owner of the target handle to
 * handle it. It is explicitly asynchronous and does not guarantee that the message will be
 * received.
 * @param[in] handle The handle to submit the message to. See @ref _DWCreateObject
 * @param[in] m The message to copy. Cannot be a @ref NullPointer.
 * @param[in] message_size Message size to copy. Currently ignored.
 * @returns @ref STATUS_OK if the message was succesfully sent, @ref STATUS_NO_ENDPOINT if the
 * target @p pid is not present, other @ref Status codes for other kinds of failures.
 */
[[gnu::nonnull(2)]]
Status _cdecl _DWIPCSend(int handle, Message *m, Size message_size);

/**
 * @brief _DWIPCReceive system call (#6) wrapper.
 * @details This routine pops a message from the process' internal queue, copies it in @p msave if
 * and only if the message was sent in the matching @p handle.
 * @param handle The handle to receive messages from. Must be between 0 and MAX_OBJ_PER_PROCESS-1
 * (Kernel define, usually 32)
 * @param[out] msave A pointer to save the message to. Must not be a NullPointer.
 * @return STATUS_RETRY if there are no messages queued. STATUS_OK if a message was found to be
 * returned. STATUS_BAD if copying the message failed.
 */
[[gnu::nonnull(2)]]
Status _cdecl _DWIPCReceive(int handle, Message *msave);

/**
 * @brief Create a new object and return a handle to it, under which handle further functionality
 * may be invoked. (System call #8)
 * @param[in] name The name of the object. If NullPointer, the object is considered private and may
 * not be shared (eg. A port may not become public)
 * @param type Type of the object. For more details, see the definitions documentation of @ref
 * ObjectType
 * @param permissions Unused and ignored for now.
 * @returns A handle to that object, under which other object-related system calls may be used, or
 * -1 if the call failed (eg. Kernel out of memory)
 * @sa _DWInvokeObject
 */
int _DWCreateObject(const char *name, ObjectType type, u32 permissions);

/**
 * @brief Invokes the functionality of the object according to the operation specified by @p op
 * (System call #9)
 * @param[in] handle Handle to the object. See return value of @ref _DWCreateObject
 * @param[in] op Operation code. See @ref DeviceObjectOp and @ref PortObjectOp for two examples.
 * @param[in,out] argptr Pointer argument for operations that may require reading/writing from a
 * pointer. Can be NullPointer, if
 * @return A status code that describes the success or failure point of the invocation.
 */
[[nodiscard]]
Status _DWInvokeObject(int handle, unsigned long op, void *argptr);

/**
 * @brief Deletes an object allocated by @ref _DWCreateObject (System call #10)
 * @warning After this function is called the object is no longer valid and may not be used.
 * @param[in] handle Handle to the object that must be deleted.
 */
void _DWDeleteObject(int handle);

DW_END_DECLS

#endif /* _KERNEL_API_H */
