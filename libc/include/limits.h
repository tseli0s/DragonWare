/**********************************************************************
 * FILE: limits.h
 * PURPOSE: Integer limits definition for DragonWare
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define INT8_MIN   ((int8_t)-128)
#define INT8_MAX   ((int8_t)127)
#define UINT8_MIN  ((uint8_t)0x00)
#define UINT8_MAX  ((uint8_t)0xff)
#define INT16_MIN  ((int16_t)-32768)
#define INT16_MAX  ((int16_t)32767)
#define UINT16_MIN ((uint16_t)0x0000)
#define UINT16_MAX ((uint16_t)0xffff)
#define UINT32_MIN ((uint32_t)0x00000000)
#define UINT32_MAX ((uint32_t)0xFFFFFFFF)
#define INT32_MIN  ((int32_t)-2147483648)
#define INT32_MAX  ((int32_t)2147483647)
#define UINT64_MIN ((uint64_t)0x0000000000000000LL)
#define UINT64_MAX ((uint64_t)0xFFFFFFFFFFFFFFFFLL)
#define INT64_MIN  ((int64_t)-9223372036854775808LL)
#define INT64_MAX  ((int64_t)9223372036854775807LL)

#define INT_MIN    ((int)INT32_MIN)
#define INT_MAX    ((int)INT32_MAX)
