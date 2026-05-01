/**********************************************************************
 * FILE: debug.h
 * PURPOSE: Debugging tools for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* Well this is close enough I don't care
 * (while (1) {} doesn't work GCC will delete everything after that) */
#define STALL_BREAKPOINT() __asm__ volatile("1: jmp 1b");

/* This will only work on Bochs. */
#define BOCHS_BREAK(loop)                           \
        {                                           \
                __asm__ volatile("xchgw %bx, %bx"); \
                if (loop) STALL_BREAKPOINT();       \
        }
