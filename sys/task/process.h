/**********************************************************************
 * FILE: process.h
 * PURPOSE: Process implementation for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "iomgr/node.h"
#include "iomgr/object.h"
#include "task.h"

/** @brief Where the user stack will be placed for each process. A high address is chosen to allow
 * the stack to grow freely if necessary in the future. Reminder that the kernel is mapped to
 * 0xC0000000-0xFFFFFFFF. */
#define DEFAULT_USER_STACK_ADDR  (0xBFFF0000)

/** @brief Maximum amount of device handles a device can have open at a given time. */
#define MAX_DEVICE_HANDLES       (32)

/**
 * @brief Maximum amount of I/O ports that a process can have.
 * @since v0.0.2
 */
#define MAX_IO_PORTS_PER_PROCESS (20)

typedef u32 ProcessID;

typedef struct _DeviceHandle {
        DeviceManagerNode *dev;
        u32                token;
} DeviceHandle;

typedef enum _ProcessCapability {
        PROC_C_NONE,
        PROC_C_SERVER,
        PROC_C_IOPL,
        PROC_C_IRQ_DISPATCH,
} ProcessCapability;

typedef struct _Process {
        Thread           *main_thread;
        u32               cr3;          /* WARNING: Physical address */
        u32               kernel_stack; /* That one's virtual, use it with SelectKernelStack */
        ProcessID         pid;
        HandleTable       handles;
        u16               ioports[MAX_IO_PORTS_PER_PROCESS];
        u16               ports_used;
        ProcessCapability flags;
        struct _Process  *prev, *next;
} Process;

/**
 * @brief Create a new process, that is ready to be scheduled (It isn't added to the scheduling
 * queue though)
 * @param[in] pid The PID of the new process. If 0, it is selected based on an internal counter.
 * @param[in] code The flat binary code that will be copied into the new process' entry point.
 * @param[in] code_size The size of that binary code to copy. Must be equivalent to the size of the
 * binary for flat binaries.
 * @returns A newly created @ref Process ready to be scheduled
 */
[[gnu::nonnull]]
Process *CreateProcess(ProcessID pid, void *code, Size code_size);

/**
 * @brief Delete a single process and free up all the memory used by it.
 * @param[in] p The process to delete
 * @warning The process must not be currently active; If the process must be freed immediately,
 * another process must take its place, to replace the paging structures used by the CPU.
 * @returns STATUS_OK if the process and all its memory has been freed succesfully, another matching
 * error code on failure.
 */
[[gnu::nonnull(1)]]
Status DeleteProcess(Process *p);

/**
 * @brief Set a process' capability flags
 * @param process The process to set the capability flags to
 * @param flags Bit field of flags to set
 */
[[gnu::nonnull(1)]]
void SetProcessCapabilities(Process *process, u32 flags);
