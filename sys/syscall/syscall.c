/**********************************************************************
 * FILE: syscall.c
 * PURPOSE: System call interface for the DragonWare kernel
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "syscall.h"

#include <atomic.h>
#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "sysquery.h"

#ifdef __i386__
#include "ddk/ia32/tss.h"
#include "ddk/ia32/vmm.h"
#endif /* __i386__ */

#include "identify.h"
#include "ipc.h"
#include "object.h"
#include "sched/schedule.h"
#include "task/process.h"
#include "task/task.h"
#include "time/timer.h"
#include "usercopy.h"

extern volatile int NeedsResched;

static void SystemIdentifySyscall(SystemIdentify *save) {
        SystemIdentify data;
        kzeromem(&data, sizeof(SystemIdentify));

        const char name[SI_MAX_NAME] = "DragonWare";
        const char tag[SI_MAX_TAG]   = DRAGONWARE_VERSION_SUFFIX;

        /* We already zero out the struct, so we don't have to fill out the
         * empty parts of the fields. */
        strncpy(data.name, name, SI_MAX_NAME);
        strncpy(data.tag, tag, SI_MAX_TAG);

        data.major    = DRAGONWARE_VERSION_MAJOR;
        data.minor    = DRAGONWARE_VERSION_MINOR;
        data.patch    = DRAGONWARE_VERSION_PATCH;
        data.build_id = __KERNEL_BUILDID__;

        (void)CopyToUser(save, &data, sizeof(SystemIdentify));
}

static Status _DWRequestPorts(const u16 *port_list, Size list_size) {
        Process *current = GetCurrentExecutionThread()->owner;
        if (!(current->flags & PROC_C_IOPL))
                return STATUS_UNSUPPORTED; /* TODO: Have a STATUS_ACCESS_REJECTED or something */

        if (list_size > MAX_IO_PORTS_PER_PROCESS) return STATUS_BAD_ARGUMENT;

        u16 ports[MAX_IO_PORTS_PER_PROCESS] = {0};
        if (CopyFromUser(ports, port_list, list_size * sizeof(u16)) != STATUS_OK)
                return STATUS_BAD_ARGUMENT;

        DisableIOPortsOfProcess(current);
        for (Size i = 0; i < list_size; i++) current->ioports[i] = ports[i];
        current->ports_used = (u16)list_size;

        EnableIOPortsOfProcess(current);
        return STATUS_OK;
}

static inline void _DWGetTicksSinceBoot(u64 *store) {
        if (!ADDRESS_IS_MAPPED(store)) return;
        u64 ticks = GetTicksSinceBoot();
        (void)CopyToUser(store, &ticks, sizeof(u64));
}

void DragonWareSyscall(SystemCallFrame *regs) {
        switch (regs->eax) {
                case SYSCALL_IDENTIFY:
                        SystemIdentifySyscall((SystemIdentify *)regs->ebx);
                        break;
                case SYSCALL_EXIT: {
                        Thread *current = GetCurrentExecutionThread();
                        RemoveThreadFromScheduler(current);
                        current->state = THREAD_TERMINATED;
                        if (current->owner) DeleteProcess(current->owner);
                        NeedsResched = 1;
                        ScheduleNext();
                        break;
                }
                case SYSCALL_YIELD:
                        YieldCurrentThread();
                        break;
                case SYSCALL_REQUEST_PORTS: {
                        ReturnFromSystemCall(regs, _DWRequestPorts, (u16 *)regs->ebx,
                                             (Size)regs->esi);
                        break;
                }
                case SYSCALL_SEND:
                        ReturnFromSystemCall(regs, _DWIPCSend, (int)regs->ebx, (Message *)regs->esi,
                                             regs->edi);
                        break;
                case SYSCALL_RECEIVE:
                        ReturnFromSystemCall(regs, _DWIPCReceive, (int)regs->ebx,
                                             (Message *)regs->esi);
                        break;
                case SYSCALL_TICK_SINCE_BOOT: {
                        _DWGetTicksSinceBoot((u64 *)regs->ebx);
                        break;
                }
                case SYSCALL_CREATE_OBJECT:
                        ReturnFromSystemCall(regs, _DWCreateObject, (const char *)regs->ebx,
                                             (ObjectType)regs->esi, regs->edi);
                        break;
                case SYSCALL_INVOKE_OBJECT:
                        ReturnFromSystemCall(regs, _DWInvokeObject, (int)regs->ebx, regs->esi,
                                             (void *)regs->edi);
                        break;
                case SYSCALL_DELETE_OBJECT:
                        _DWDeleteObject((int)regs->ebx);
                        break;
                case SYSCALL_TRANSLATE_HANDLE:
                        ReturnFromSystemCall(regs, _DWTranslateHandle, (ProcessID)regs->ebx,
                                             (int)regs->esi, (int *)regs->edi);
                        break;
                case SYSCALL_SYSTEM_QUERY:
                        ReturnFromSystemCall(regs, _DWSystemQuery, (SystemQuery)regs->ebx,
                                             (void *)regs->esi);
                        break;
                default:
                        regs->eax = (u32)STATUS_BAD_SYSCALL;
                        break;
        }
}
