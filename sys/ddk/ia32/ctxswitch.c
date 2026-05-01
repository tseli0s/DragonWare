/**********************************************************************
 * FILE: ctxswitch.c
 * PURPOSE: Context switching platform specific code
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "ctxswitch.h"

#include "gdt.h"
#include "vmm.h"

void SwapProcess(Process *new) {
        SetPageDirectory(new->main_thread->owner->cr3);
        SelectKernelStack(new->main_thread->owner->kernel_stack);
}
