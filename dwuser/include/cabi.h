/**********************************************************************
 * FILE: cabi.h
 * PURPOSE: C function ABI helpers and definitions
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef _C_STD_H
#define _C_STD_H 1

#include "cppsupport.h"

DW_BEGIN_DECLS

#ifdef __GNUC__
#define _cdecl    __attribute__((cdecl))
#define _fastcall __attribute__((fastcall))
#define noreturn  __attribute__((noreturn))
#endif /* __GNUC__ */

DW_END_DECLS

#endif /* _C_STD_H */
