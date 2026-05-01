/**********************************************************************
 * FILE: dlibc_common.h
 * PURPOSE: Common definitions used in DragonWare C Library's implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifdef __cplusplus
#define DLC_BEGIN_DECLS extern "C" {
#define DLC_END_DECLS   }
#else
#define DLC_BEGIN_DECLS
#define DLC_END_DECLS
#endif /* __cplusplus */
