/**********************************************************************
 * FILE: mmutils.c
 * PURPOSE: Memory manipulation/reading functions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>

void *memmove(void *dest, const void *src, Size n) {
        Byte       *d = dest;
        const Byte *s = src;

        if (d < s) {
                while (n--) *d++ = *s++;
        } else {
                d += n;
                s += n;
                while (n--) *--d = *--s;
        }
        return dest;
}
