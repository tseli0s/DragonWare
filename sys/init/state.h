/**********************************************************************
 * FILE: state.h
 * PURPOSE: Kernel internal state and runtime settings
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* Kernel state. Will be useful in the future to enable/disable features
 * directly from init time. */
typedef struct _InternalKernelState {
        Bool init;
        int  loglevel;
} InternalKernelState;

InternalKernelState *GetKernelState(void);
