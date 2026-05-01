/**********************************************************************
 * FILE: spinlock.h
 * PURPOSE: Simple spinlock implementation for safe shared state access/write operations
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef DW_SPINLOCK_TIMEOUT
#define DW_SPINLOCK_TIMEOUT (10000000ULL)
#endif /* DW_SPINLOCK_TIMEOUT */

#include <ktypes.h>
#include <macros.h>

typedef struct _Spinlock {
        volatile Bool locked;
} Spinlock;

static inline void AcquireSpinlock(Spinlock *s) {
        /* pause is common for spinlocks, so I'll use that over hlt */
        while (unlikely(__sync_lock_test_and_set(&s->locked, 1))) __asm__ volatile("pause");
}

static inline void ReleaseSpinlock(Spinlock *s) { __sync_lock_release(&s->locked); }
