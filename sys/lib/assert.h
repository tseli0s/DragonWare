/**********************************************************************
 * FILE: assert.h
 * PURPOSE: Simple assertion macro(s)
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once
#include <panic.h>

/* Assertions are only enabled in debug mode */
#ifdef DRAGONWARE_DEBUG_MODE
#define kassert(condition)                                                                    \
        do {                                                                                  \
                if (!(condition)) {                                                           \
                        FatalError("Bad kernel assertion: " #condition " at " __FILE_NAME__); \
                }                                                                             \
        } while (0)
#else
#define kassert(condition) ((void)sizeof(condition))
#endif
