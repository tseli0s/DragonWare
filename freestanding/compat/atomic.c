/**********************************************************************
 * FILE: atomic.c
 * PURPOSE: Atomic copying/moving based on GNU C compiler builtins
 * PROJECT: DragonWare Freestanding Library
 * DATE: 10-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "atomic.h"

void memcpy_atomic(void *dst, const void *src, Size size) {
        Size i    = 0;
        Size step = sizeof(Size);

        for (; i + step <= size; i += step) {
                Size tmp = 0;
                __atomic_load((const Size *)((const char *)src + i), &tmp, __ATOMIC_SEQ_CST);
                __atomic_store((Size *)((char *)dst + i), &tmp, __ATOMIC_SEQ_CST);
        }

        for (; i < size; i++) {
                char tmp = 0;
                __atomic_load((const char *)src + i, &tmp, __ATOMIC_SEQ_CST);
                __atomic_store((char *)dst + i, &tmp, __ATOMIC_SEQ_CST);
        }
}

void memmove_atomic(void *dst, const void *src, Size size) {
        if (dst < src || (Byte *)dst >= (Byte *)src + size)
                memcpy_atomic(dst, src, size);
        else {
                Size i = size;

                while (i > 0) {
                        i--;
                        char tmp;
                        __atomic_load((const char *)src + i, &tmp, __ATOMIC_SEQ_CST);
                        __atomic_store((char *)dst + i, &tmp, __ATOMIC_SEQ_CST);
                }
        }
}
