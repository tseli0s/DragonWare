/**********************************************************************
 * FILE: stddef.h
 * PURPOSE: Standard libc type definitions
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "dlibc_common.h"
#include "stdint.h"

DLC_BEGIN_DECLS

#ifdef __GNUC__

typedef __SIZE_TYPE__    size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;

#endif /* __GNUC__ */

typedef struct {
        long long   __ll;
        long double __ld;
} max_align_t;

#define NULL                   ((void *)0)
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

typedef uint32_t wchar_t;

DLC_END_DECLS
