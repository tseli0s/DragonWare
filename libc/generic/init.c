/**********************************************************************
 * FILE: init.c
 * PURPOSE: libc internal initialization routines
 * PROJECT: DragonWare C Library
 * DATE: 05-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>

#include "syscalls/syscall86.h"

/* XXX We can't use the dwuser library here - We would have a circular dependency. So we have to
 * perform the system calls manually. Luckily I don't plan on moving system calls around, so
 * this will work for some time, although fragile.*/

[[gnu::visibility("hidden")]]
int __dlibc_console_handle = -1;

static void __libc_register_handles(void) {
        const char *console_port_name = "CONSOLE";

        /* Keep retrying until the kernel creates the object */
        while (__dlibc_console_handle < 0)
                __dlibc_console_handle = (int)__make_syscall_ia32_3param_reti32(
                        SYSCALL_CREATE_OBJECT, 0, OBJ_PORT, 0);

        /* Same thing here - Keep trying to open the console until it comes up */
        Status invoke_status = (Status)__make_syscall_ia32_3param_reti32(
                SYSCALL_INVOKE_OBJECT, (uint32_t)__dlibc_console_handle, PORT_OPEN,
                (uint32_t)console_port_name);

        /* Usually this means it is during very early boot and the console port isn't available (Or
         * even that it's the console port itself being loaded). Normally we'd spin here, but then
         * the whole system deadlocks if the console can't be started, so move on without output. */
        if (invoke_status != STATUS_OK) __dlibc_console_handle = -1;
}

void __libc_init_internal(void) { __libc_register_handles(); }

[[gnu::visibility("hidden")]]
void __libc_cleanup(void) {
        __make_syscall_ia32_1param(SYSCALL_DELETE_OBJECT, (uint32_t)__dlibc_console_handle);
}
