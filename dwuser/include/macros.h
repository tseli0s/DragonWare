/**********************************************************************
 * FILE: macros.h
 * PURPOSE: Macro helpers for DragonWare userland applications
 * PROJECT: DragonWare User Library
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>

#ifndef arraysize
/**
 * @brief Returns the size of an array in elements
 * @warning @p arr must not be a pointer, but a regular C-style array
 * @since v0.0.2
 */
#define arraysize(__arr) ((Size)(sizeof((__arr))) / sizeof(*__arr))
#endif /* arraysize */

#ifndef min
/**
 * @brief Returns the minimum of two elements given
 * @param __a First element
 * @param __b Second element
 * @since v0.0.2
 */
#define min(__a, __b) (((__a) > (__b)) ? (__b) : (__a))
#endif /* min */

#ifndef max
/**
 * @brief Returns the maximum of two elements given
 * @param __a First element
 * @param __b Second element
 * @since v0.0.2
 */
#define max(__a, __b) (((__a) > (__b)) ? (__a) : (__b))
#endif /* max */
