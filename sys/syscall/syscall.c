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

        CopyToUser(save, &data, sizeof(SystemIdentify));
}

static void _DWklog(int level, const char *msg) {
        char buf[LOG_MAXBUF];
        ZeroMemory(buf);
        if (CopyFromUser(buf, msg, strnlen(msg, LOG_MAXBUF)) != 0) return;
        klog((LogLevel)level, "%s", buf);
}

/* TODO: Also note the replacement function (Probably by an I/O object?) */
[[deprecated(
        "_DWRaiseIOPL is no longer available, the TSS I/O bitmap has replaced its functionality")]]
static Status _DWRaiseIOPL(u32 *eflags) {
        /* set the IOPL bit in eflags, but only if the current process possesses the
         * capability to actually use that. */
        if (GetCurrentExecutionThread()->owner->flags & PROC_C_IOPL) {
                *eflags |= (3 << 12);
                return STATUS_OK;
        } else
                return STATUS_UNSUPPORTED;
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
                case SYSCALL_KLOG:
                        _DWklog((int)regs->ebx, (const char *)regs->esi);
                        break;
                case SYSCALL_RAISE_IOPL: {
                        regs->eax = (u32)STATUS_BAD_SYSCALL;
                        break;
                }
                case SYSCALL_SEND:
                        regs->eax =
                                (u32)_DWIPCSend((int)regs->ebx, (Message *)regs->esi, regs->edi);
                        break;
                case SYSCALL_RECEIVE:
                        regs->eax = (u32)_DWIPCReceive((int)regs->ebx, (Message *)regs->esi);
                        break;
                case SYSCALL_TICK_SINCE_BOOT: {
                        /* System V ABI says that, for a 64 bit return value, high 32 bits go in
                         * edx, and the low 32 bits go in eax. So we can simply mask out the high 32
                         * bits when assigning to eax and shift down edx by the amount of low
                         * bits.
                         * FIXME: This is broken on sysenter/sysexit instructions (edx must be
                         * preserved, it holds the return address to userland).
                         * */
                        u64 ticks = GetTicksSinceBoot();
                        regs->eax = (u32)(ticks & 0xffffffff);
                        // regs->edx = (u32)(((u64)ticks) >> 32);
                        break;
                }
                case SYSCALL_CREATE_OBJECT:
                        regs->eax = (u32)_DWCreateObject((const char *)regs->ebx,
                                                         (ObjectType)regs->esi, regs->edi);
                        break;
                case SYSCALL_INVOKE_OBJECT:
                        regs->eax =
                                (u32)_DWInvokeObject((int)regs->ebx, regs->esi, (void *)regs->edi);
                        break;
                case SYSCALL_DELETE_OBJECT:
                        _DWDeleteObject((int)regs->ebx);
                        break;
                default:
                        regs->eax = (u32)STATUS_BAD_SYSCALL;
                        break;
        }
}

void POSIXSyscall(SystemCallFrame *regs) {
        LogMessage(LOG_WARNING,
                   "POSIX syscall invoked. POSIX compatibility has not been implemented "
                   "yet.");
        regs->eax = (u32)STATUS_BAD_SYSCALL;
}
