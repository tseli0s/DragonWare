/**********************************************************************
 * FILE: stdint.h
 * PURPOSE: stdint.h libc header implementation, providing fixed-width integer implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "dlibc_common.h"

/* To prevent including machine-specific headers directly only */
#define __DLIBC_STDINT_H__ 1

DLC_BEGIN_DECLS

#ifdef __i386__
#include "machine/ia32/_stdint.h"
#else
#error Unsupported target for DragonWare C Library
#endif /* __i386__ */

DLC_END_DECLS
