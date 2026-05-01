/**********************************************************************
 * FILE: stdbool.h
 * PURPOSE: stdbool.h libc header implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#if __STDC_VERSION__ < 202311L
/* C23 now has these types within the language, they're not necessary */

#define bool  int
#define true  (1)
#define false (0)
#endif /* __STDC_VERSION__ */
