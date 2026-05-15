/**********************************************************************
 * FILE: syscall.c
 * PURPOSE: C implementation of interfaces to DragonWare system calls
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>

#include "syscalls/syscall86.h"

void _DWSystemIdentify(SystemIdentify *saveptr) {
        __make_syscall_ia32_1param(SYSCALL_IDENTIFY, (u32)saveptr);
}

void _cdecl noreturn _DWExit(void) {
        __make_syscall_ia32_0param(SYSCALL_EXIT);
        __builtin_unreachable();
}

void _cdecl _DWYield(void) { __make_syscall_ia32_0param(SYSCALL_YIELD); }

void _cdecl _DWklog(LogLevel level, const char *msg) {
        __make_syscall_ia32_2param(SYSCALL_KLOG, (u32)level, (u32)msg);
}

#if 0
Status _cdecl _DWRaiseIOPL(void) {
        return (Status)__make_syscall_ia32_0param_reti32(SYSCALL_RAISE_IOPL);
}
#else
Status _cdecl _DWRequestPorts(const u16 *port_list, Size port_list_size) {
        return (Status)__make_syscall_ia32_2param_reti32(SYSCALL_REQUEST_PORTS, (uint32_t)port_list,
                                                         (uint32_t)port_list_size);
}
#endif /* DRAGONWARE_VERSION_PATCH */

Status _cdecl _DWIPCSend(int handle, Message *m, Size message_size) {
        return __make_syscall_ia32_3param_reti32(SYSCALL_SEND, handle, (u32)m, (u32)message_size);
}

Status _cdecl _DWIPCReceive(int handle, Message *msave) {
        return (Status)__make_syscall_ia32_2param_reti32(SYSCALL_RECEIVE, handle, (u32)msave);
}

int _DWCreateObject(const char *name, ObjectType type, u32 permissions) {
        return (int)__make_syscall_ia32_3param_reti32(SYSCALL_CREATE_OBJECT, (u32)name, type,
                                                      permissions);
}

[[nodiscard]]
Status _DWInvokeObject(int handle, unsigned long op, void *argptr) {
        return (Status)__make_syscall_ia32_3param_reti32(SYSCALL_INVOKE_OBJECT, handle, op,
                                                         (u32)argptr);
}

void _DWDeleteObject(int handle) { __make_syscall_ia32_1param(SYSCALL_DELETE_OBJECT, handle); }
