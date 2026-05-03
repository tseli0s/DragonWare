/**********************************************************************
 * FILE: errno.c
 * PURPOSE: C errno code support internal implementations
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

[[gnu::visibility("default")]]
volatile int errno = 0;

int __get_errno(void) { return errno; }
