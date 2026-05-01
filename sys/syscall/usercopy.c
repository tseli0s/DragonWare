/**********************************************************************
 * FILE: usercopy.h
 * PURPOSE: Userland to kernel buffer/pointer copying safety guards
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "usercopy.h"

#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "ddk/ia32/vmm.h"

static inline Bool IsUserPointer(uintptr_t addr) {
        /* Okay, not perfect, but for now it'll work. For this to work, the kernel must not map its
         * own memory below KERNEL_VM_BASE of course, otherwise this is a bogus check. */
        return addr < KERNEL_VM_BASE;
}

static inline Bool AdditionWouldOverflow(u32 n1, u32 n2) {
        /* This only works with unsigned integers - Signed overflow is a separate beast. */
        return (n1 + n2) < n1;
}

Status CopyFromUser(void *restrict dest, const void *restrict src, Size n_bytes) {
        const uintptr_t useraddr    = (uintptr_t)src;
        const uintptr_t enduseraddr = useraddr + n_bytes;
        if (!ADDRESS_IS_MAPPED(useraddr) || !ADDRESS_IS_MAPPED(enduseraddr))
                return STATUS_BAD_ARGUMENT;

        if (unlikely(AdditionWouldOverflow(useraddr, n_bytes))) return STATUS_BAD;

        if (!IsUserPointer(useraddr) || !IsUserPointer(enduseraddr))
                return STATUS_BAD; /* What are you doing bad process? */

        memcpy(dest, src, n_bytes);
        return STATUS_OK;
}

Status CopyToUser(void *restrict dest, const void *restrict src, Size n_bytes) {
        const uintptr_t useraddr    = (uintptr_t)dest;
        const uintptr_t enduseraddr = useraddr + n_bytes;

        if (!ADDRESS_IS_MAPPED(useraddr) || !ADDRESS_IS_MAPPED(enduseraddr))
                return STATUS_BAD_ARGUMENT;

        if (unlikely(AdditionWouldOverflow(useraddr, n_bytes))) return STATUS_BAD;
        if (!IsUserPointer(useraddr) || !IsUserPointer(enduseraddr)) return STATUS_BAD;

        memcpy(dest, src, n_bytes);
        return STATUS_OK;
}
