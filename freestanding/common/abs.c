/**********************************************************************
 * FILE: abs.c
 * PURPOSE: Absolute function definitions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 11-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "abs.h"

#include "ktypes.h"
#include "limits.h"

int abs(int n) {
        if (n >= 0 || n == INT_MIN) return n;

        /* Consider it practice for my logical design class, I think it works for all negative
         * integers. */
        return ~n + 1;
}

long labs(long n) {
        if (n >= 0 || n == LONG_MIN) return n;

        /* Consider it practice for my logical design class, I think it works for all negative
         * integers. */
        return ~n + 1;
}

long long llabs(long long n) {
        if (n >= 0 || n == LLONG_MIN) return n;

        /* Consider it practice for my logical design class, I think it works for all negative
         * integers. */
        return ~n + 1;
}
