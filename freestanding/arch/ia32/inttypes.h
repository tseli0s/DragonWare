/**********************************************************************
 * FILE: inttypes.h
 * PURPOSE: Fixed width integer types for DragonWare freestanding components
 * PROJECT: DragonWare Freestanding Library
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifdef INCLUDE_STDINT_H_COMPILER /* This will be a feature flag in the future */
#include <stdint.h>
#else

/* --------------- SIGNED INTEGERS ----------------- */
typedef signed char      int8_t;
typedef signed short     int16_t;
typedef signed int       int32_t;
typedef signed long long int64_t;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* ------------- UNSIGNED INTEGERS -------------------- */
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* -------------- MISCELLANEOUS --------------------- */
typedef uint32_t size_t;
typedef int32_t  ssize_t;
typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;

/* About off_t: After defining it for months as a signed 32 bit integer,
 * I tried "upgrading" it to a 64 bit integer, and everything broke down.
 * We're stuck with the 32 bits for file offsets for now.
 */
typedef int32_t off_t;

#endif /* INCLUDE_STDINT_H_COMPILER */
