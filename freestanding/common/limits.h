/**********************************************************************
 * FILE: limits.h
 * PURPOSE: Integer limits definitions for DragonWare freestanding environment
 * PROJECT: DragonWare Freestanding Library
 * DATE: 05-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define I8_MIN  ((i8)(-128))
#define I8_MAX  ((i8)127)
#define U8_MIN  ((u8)0x00)
#define U8_MAX  ((u8)0xFF)
#define I16_MIN ((i16)(-32768))
#define I16_MAX ((i16)32767)
#define U16_MIN ((u16)0x0000)
#define U16_MAX ((u16)0xFFFF)
#define U32_MIN ((u32)0x00000000)
#define U32_MAX ((u32)0xFFFFFFFF)
#define I32_MIN ((i32)(-2147483648))
#define I32_MAX ((i32)2147483647)
#define U64_MIN ((u64)0x0000000000000000LL)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFFLL)
#define I64_MIN ((i64)(-9223372036854775808LL))
#define I64_MAX ((i64)9223372036854775807LL)

#define INT_MIN ((int)I32_MIN)
#define INT_MAX ((int)I32_MAX)
