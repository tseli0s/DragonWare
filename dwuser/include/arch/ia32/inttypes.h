/**********************************************************************
 * FILE: inttypes.h
 * PURPOSE: Reexports of freestanding library types to userland applications
 * PROJECT: DragonWare User Library
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/*
 * NOTE: The intN_t types aren't defined here,
 * instead they're defined in libc since DragonWare has its own
 * way of defining such types.
 */

#pragma once

/* --------------- SIGNED INTEGERS ----------------- */
typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
typedef signed long long i64;

/* ------------- UNSIGNED INTEGERS -------------------- */
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
