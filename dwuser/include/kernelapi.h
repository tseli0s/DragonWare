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
#define _KERNEL_API_H            1

#define SYSCALL_IDENTIFY         (0)
#define SYSCALL_EXIT             (1)
#define SYSCALL_YIELD            (2)
#define SYSCALL_KLOG             (3)
#define SYSCALL_REQUEST_PORTS    (4)
#define SYSCALL_SEND             (5)
#define SYSCALL_RECEIVE          (6)
#define SYSCALL_TICK_SINCE_BOOT  (7)
#define SYSCALL_CREATE_OBJECT    (8)
#define SYSCALL_INVOKE_OBJECT    (9)
#define SYSCALL_DELETE_OBJECT    (10)
#define SYSCALL_TRANSLATE_HANDLE (11)
#define SYSCALL_SYSTEM_QUERY     (12)

#include "cabi.h"
#include "cppsupport.h"
#include "ipc86.h"
#include "kerneltypes.h"

#define SI_MAX_NAME              (24)
#define SI_MAX_TAG               (12)
#define MAX_IO_PORTS_PER_PROCESS (20)

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
 * @since v0.0.1
 */
typedef struct [[gnu::packed]] _DeviceMapDescriptor {
        u32 irq;       /** < IRQ occupied by this device. If 0, this device does not use IRQs. */
        u64 mmio_addr; /** < Address where this device is mapped. If 0, this device cannot be
                          accessed through MMIO. */
        u64 mmio_len;  /** < Length of the physical address of this device's mapping. Only valid if
                          mmio_addr is also valid. */
        struct [[gnu::packed]] {
                u32 width;
                u32 height;
                u32 bpp;
                u32 stride;
        } fb; /** < Framebuffer information. Only valid for framebuffer devices (Like the
                 kernel-arbited "Kernel Framebuffer"). Only for versions v0.0.2 of DragonWare and
                 later. */
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

/**
 * @brief Data describing thread initial state for use with thread objects.
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _UserThreadData {
        void  (*entry)(void *); /** << Entry point of a thread  */
        void *stack;            /** << Stack memory for the thread*/
        void *extra_data;       /** << Extra data that will be given to the thread upon execution.*/
} UserThreadData;

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
        OBJ_THREAD,      /** << Thread (Unit of execution) object */
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
 * @brief An operation to be performed on a thread object.
 * @since v0.0.2
 */
typedef enum _ThreadObjectOp : unsigned long {
        THREAD_CREATE, /** << Create a new thread */
        THREAD_RUN,    /** << Enlist the thread in the scheduler */
} ThreadObjectOp;

/**
 * @brief A system value used by the kernel. Usually referring to constants like page size, amount
 * of CPUs that can be used, and others.
 * @since v0.0.2
 */
typedef enum _SystemQuery : u32 {
        SQ_NONE                  = 0,
        SQ_N_OBJECTS_PER_PROCESS = 1, /** << Amount of objects per process  */
        SQ_PAGE_SIZE = 2, /** << Page size used by the kernel's virtual memory manager */
        SQ_N_CPUS    = 3, /** << Reserved  */
        SQ_TIMER_HZ =
                4, /** << Clock rate of the timer used by the kernel to perform quantum tracking */
        SQ_DEBUG_BUILD = 5, /** Whether this is a debug build or not  */
} SystemQuery;

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
 * @brief _DWRequestPorts system call (#4) wrapper.
 * @details This function requests from the kernel permission to directly read from the I/O ports
 * specified inside @p port_list, an array of 16-bit integers of ports that the process needs to
 * talk to. It is only relevant for x86-based systems at the moment.
 * @param[in] port_list An array of ports that the process wants to talk to. Elements beyond @ref
 * MAX_IO_PORT_PER_PROCESS will be ignored.
 * @param[in] port_list_size The amount of ports to read from the array.
 * @returns STATUS_OK if the process has been granted the ability to talk to ports. Other @ref
 * Status codes if it failed, depending on the reason of failure.
 * @since v0.0.2
 */
Status _cdecl _DWRequestPorts(const u16 *port_list, Size port_list_size);

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
 * @brief _DWGetTicksSinceBoot system call (#7) wrapper.
 * @details This function will write in the memory address pointed to by @p store the amount of
 * clock ticks since the system was booted. The clock is configured at build time to tick at 100Hz
 * (see sys/time/timer.c, the define @ref TARGET_HZ)
 * @note A negligible amount of inaccuracy exists - The kernel does not record ticks since it was
 * first loaded into memory. This leads to a loss of a few ticks (depending on the speed of the
 * central processing unit and kernel optimizations), usually but not definitively around 4-5 ticks.
 * @param[in] store Where to store the tick value. Must point to an eight byte block of memory,
 * writeable by the caller.
 */
[[gnu::nonnull]]
void _cdecl _DWGetTicksSinceBoot(u64 *store);

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

/**
 * @brief Duplicates a handle of another process to the caller process. (System call #11)
 * @details Given a handle @p handle belonging to a process by ID @p process_id, duplicate the
 * handle and store it into @p save for the caller process.
 * @param process_id The ID of the process that @p handle belongs to. 0 is invalid.
 * @param handle The handle of the other process to translate into the current process.
 * @param[out] save Where to store the resulting handle on success. This is a user pointer.
 * @note The @p handle is translated and appended into the current process' handle table. The new
 * handle stored in @p save may be different than the original @p handle
 * @returns STATUS_OK if the handle was translated successfully. STATUS_NOT_FOUND if the handle does
 * not point to anything valid or the process with ID @p process_id is not found.
 * STATUS_BAD_ARGUMENT if one of the parameters is invalid. STATUS_OUT_OF_MEMORY if there are no
 * free slots to store the handle to. STATUS_BAD if the copy to @p save failed.
 */
Status _DWTranslateHandle(ProcessID process_id, int handle, int *save);

/**
 * @brief Queries a system value @p key and returns the kernel-configured value for it. If extra
 * data needs to be copied, it is copied to the memory address pointed to by @p store (Currently
 * unused as of v0.0.2)
 * @param key The system value to query. See @ref SystemQuery
 * @param[out] store Pointer to memory where extra data will be copied, if needed, otherwise
 * ignored.
 * @returns The value of @p key as a system value upon the time of the call.
 */
int _DWSystemQuery(SystemQuery key, void *store);

/**
 * @brief Returns a string describing the meaning of @p status_code
 * @param status_code The status code to describe.
 * @returns A human readable string describing in English the meaning of @p status_code
 * @warning The returned pointer is read only. Modifying it can cause undefined behaviour.
 * @sa @ref Status
 * @since v0.0.2
 */
[[gnu::returns_nonnull]]
const char *StringifyStatus(Status status_code);

DW_END_DECLS

#endif /* _KERNEL_API_H */
