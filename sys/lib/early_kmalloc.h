/**********************************************************************
 * FILE: early_kmalloc.h
 * PURPOSE: Static memory allocation for DragonWare, used to bring up the actual allocator later
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/* AllocateStaticMemory works by returning pointers to static memory (global one,
 * though), only to be used to bring up the actual memory allocator. DON'T use
 * this anywhere else.
 *
 * Note that you can't free anything allocated by AllocateStaticMemory  */
void *AllocateStaticMemory(Size size);
