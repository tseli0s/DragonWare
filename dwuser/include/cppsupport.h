/**********************************************************************
 * FILE: cppsupport.h
 * PURPOSE: C++ language support for the DragonWare library
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef _CPP_SUPPORT_H
#define _CPP_SUPPORT_H 1

#ifdef __cplusplus
#define DW_BEGIN_DECLS extern "C" {
#define DW_END_DECLS   }
#else /* __cplusplus */
#define DW_BEGIN_DECLS
#define DW_END_DECLS
#endif /* __cplusplus */

#endif /* _CPP_SUPPORT_H */
