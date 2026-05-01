/**********************************************************************
 * FILE: state.c
 * PURPOSE: Kernel internal state and runtime settings
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "state.h"

static InternalKernelState state = {0};

/* This will be called before the kernel is started, don't touch it. */
void default_state(void) {
        state.init     = true;
        state.loglevel = 0;
}

InternalKernelState *GetKernelState(void) { return &state; }
