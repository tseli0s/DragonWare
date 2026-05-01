/**********************************************************************
 * FILE: atomic.h
 * PURPOSE: Atomic functions declarations
 * PROJECT: DragonWare Freestanding Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* Atomic versions of memory functions */

void memcpy_atomic(void *dst, const void *src, Size size);
void memmove_atomic(void *dst, const void *src, Size size);
