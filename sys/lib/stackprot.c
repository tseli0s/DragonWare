/**********************************************************************
 * FILE: stackprot.c
 * PURPOSE: Stack smashing protection support for GCC -fstack-protector option implementation
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>
#include <macros.h>
#include <panic.h>

/* TODO: We should randomize this per build for greater security, when we have userspace support
 * running of course */
#ifndef STACK_CHECK_GUARD
#define STACK_CHECK_GUARD (0xa4dc10be)
#endif /* STACK_CHECK_GUARD */

uintptr_t __stack_chk_guard = STACK_CHECK_GUARD;

[[noreturn]]
void __stack_chk_fail(void) {
        FatalError("Stack smashing detected (Magic value mismatch: %d)", __stack_chk_guard);
        __builtin_unreachable();
}
