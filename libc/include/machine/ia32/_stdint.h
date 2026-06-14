/**********************************************************************
 * FILE: _stdint.h
 * PURPOSE: IA32-dependent integer types implementation
 * PROJECT: DragonWare C Library
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef __DLIBC_STDINT_H__ /* Defined in libc/include/stdint.h */
#error Do not include machine-dependent _stdint.h directly!
#endif /* __DLIBC_STDINT_H__ */

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

typedef int8_t   int_least8_t;
typedef uint8_t  uint_least8_t;
typedef int16_t  int_least16_t;
typedef uint16_t uint_least16_t;
typedef int32_t  int_least32_t;
typedef uint32_t uint_least32_t;
typedef int64_t  int_least64_t;
typedef uint64_t uint_least64_t;

typedef int32_t  int_fast8_t;
typedef int32_t  int_fast16_t;
typedef int32_t  int_fast32_t;
typedef int64_t  int_fast64_t;
typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;

/* Normally these are defined in stddef.h however because they differ by architecture and this file
 * is specifically here for this purpose we'll define them here and include them in stddef.h */
typedef signed long ptrdiff_t;
typedef uint32_t    size_t;
typedef int32_t     ssize_t;
