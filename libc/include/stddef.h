/**********************************************************************
 * FILE: stddef.h
 * PURPOSE: Standard libc type definitions
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define __STDC_VERSION_STDDEF_H__ 202311L

#include "dlibc_common.h"
#include "stdint.h"

DLC_BEGIN_DECLS

#if __STDC_VERSION__ < 202311L
#define NULL ((void *)0)
#else
#define NULL nullptr
#endif /* __STDC_VERSION__ */

#define unreachable() ((void)__builtin_unreachable())

typedef struct {
        long long   __ll;
        long double __ld;
} max_align_t;

#define offsetof(type, member) ((size_t)&(((type *)0)->member))

typedef uint32_t wchar_t;
typedef typeof_unqual(nullptr) nullptr_t;

DLC_END_DECLS
