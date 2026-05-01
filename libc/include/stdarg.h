/**********************************************************************
 * FILE: stdarg.h
 * PURPOSE: stdarg.h libc header definitions
 * PROJECT: DragonWare C Library
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#if defined(__GNUC__) || defined(__clang__)
typedef __builtin_va_list va_list;

#define va_start(ap, last)   __builtin_va_start((ap), (last))
#define va_arg(ap, type)     __builtin_va_arg((ap), type)
#define __va_copy(dest, src) __builtin_va_copy((dest), (src))
#define va_end(ap)           __builtin_va_end(ap)

#else
#error This compiler cannot be used to compile the DragonWare C Library, as it lacks the necessary builtins to provide va_start()
#endif /* __GNUC__ */
