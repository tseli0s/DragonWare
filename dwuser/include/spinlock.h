/**********************************************************************
 * FILE: spinlock.h
 * PURPOSE: Simple spinlock helper for DragonWare applications
 * PROJECT: DragonWare User Library
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kernelapi.h>

#ifndef __GNUC__
#warning Compiler not detected to be GCC, spinlock support may be shaky
#endif /* __GNUC__ */

/**
 * @brief Simple spinlock implementation in dwuser
 * @since v0.0.2
 */
typedef struct _Spinlock {
        volatile int __locked;
} Spinlock;

/**
 * @brief Attempt to acquire (lock) the spinlock @p s given or yield if it is currently used by
 * another procedure.
 * @param[in] s The spinlock to acquire, must not be a @ref NullPointer
 * @since v0.0.2
 * @sa AcquireSpinlockBusy
 */
[[gnu::nonnull]]
static inline void AcquireSpinlock(Spinlock *s) {
#ifdef __GNUC__
        while (__sync_lock_test_and_set(&s->__locked, 1)) _DWYield();
#else
#error Cannot implement atomic test and set without GCC builtins
#endif /* __GNUC__ */
}

/**
 * @brief Attempt to acquire (lock) the spinlock @p s given and busywait until it becomes available (ie. No longer held by any other procedure)
 * @param[in] s The spinlock to acquire, must not be a @ref NullPointer
 * @since v0.0.2
 * @sa AcquireSpinlock
 */
[[gnu::nonnull]]
static inline void AcquireSpinlockBusy(Spinlock *s) {
#ifdef __GNUC__
        while (__sync_lock_test_and_set(&s->__locked, 1)) __asm__ volatile("pause");
#else
#error Cannot implement atomic test and set without GCC builtins
#endif /* __GNUC__ */
}

/**
 * @brief Release a held spinlock @p s and allow another procedure to access it.
 * @param[in] s The spinlock to release, must not be a @ref NullPointer
 * @since v0.0.2
 * @sa AcquireSpinlock
 */
[[gnu::nonnull]]
static inline void ReleaseSpinlock(Spinlock *s) {
#ifdef __GNUC__
        __sync_lock_release(&s->__locked);
#else
#error Cannot implement atomic release without GCC builtins
#endif /* __GNUC__ */
}
