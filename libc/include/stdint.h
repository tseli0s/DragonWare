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

DLC_BEGIN_DECLS

#ifdef __GNUC__

typedef __INT8_TYPE__  int8_t;
typedef __UINT8_TYPE__ uint8_t;

typedef __INT16_TYPE__  int16_t;
typedef __UINT16_TYPE__ uint16_t;

typedef __INT32_TYPE__  int32_t;
typedef __UINT32_TYPE__ uint32_t;

typedef __INT64_TYPE__  int64_t;
typedef __UINT64_TYPE__ uint64_t;

#else
/* Doing some guesswork here, not very portable if you ask me
 * so when we go 64 bits this has to be changed */
typedef signed char            int8_t;
typedef unsigned char          uint8_t;
typedef signed short           int16_t;
typedef unsigned short         uint16_t;
typedef signed int             int32_t;
typedef unsigned int           uint32_t;
typedef signed long long int   int64_t;
typedef unsigned long long int uint64_t;

#endif /* __GNUC__ */

#if defined(__i386__) /* Valid on i386 only */
typedef signed long int   intptr_t;
typedef unsigned long int uintptr_t;
#else
typedef signed long long int   intptr_t;
typedef unsigned long long int uintptr_t;
#endif /* __i386__ */

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

DLC_END_DECLS
