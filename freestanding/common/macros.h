/**********************************************************************
 * FILE: macros.h
 * PURPOSE: Helper macros and compiler-dependent definitions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* Just some helpful macros for clearer syntax. You can also do all this
 * manually. */

/**
 * @brief Returns the offset of the member inside a struct in bytes.
 * @since v0.0.2
 */
#define offsetof(_t, _m)         __builtin_offsetof(_t, _m)

/**
 * @brief Check if a value lies within a closed interval.
 * @param _num Value to test.
 * @param _low Lower bound (inclusive).
 * @param _up Upper bound (inclusive).
 * @returns True if _num is between _low and _up, otherwise false.
 */
#define inrange(_num, _low, _up) (((_num) <= (_up) && (_num) >= (_low)))

/**
 * @brief Compute the maximum of two values.
 */
#define max(_a, _b)              (((_a) > (_b)) ? (_a) : (_b))

/**
 * @brief Compute the minimum of two values.
 */
#define min(_a, _b)              (((_a) > (_b)) ? (_b) : (_a))

/**
 * @brief Sets a given memory range to 0, automatically trying to guess the size of that range using
 * sizeof()
 * @note Doesn't work on pointers, use @ref kzeromem to control the size. This macro runs sizeof()
 * on @p obj and the size of a pointer is only 4-8 bytes.
 */
#define ZeroMemory(obj)          kzeromem(obj, sizeof(obj))

/** @brief Computes the size of a static array by dividing its total size by the size of each
 * element  */
#define arraysize(arr)           (sizeof(arr) / sizeof(arr[0]))

/** @brief Determine if a year is a leap year. This uses the Gregorian calendar rules. */
#define isleap(year)             ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)

/**
 * @brief Align a value upwards to the nearest alignment boundary.
 * @param x Value to align.
 * @param align Alignment boundary (must be non-zero).
 * @returns The smallest value >= x that is a multiple of align.
 */
#define alignup(x, align)        (((Size)(x) + ((Size)(align) - 1)) / (Size)(align) * (Size)(align))

/**
 * @brief Align a value upwards to the PAGE_SIZE boundary (usually 4096).
 * @param _x Value to align.
 * @returns The smallest value >= x that is page aligned.
 */
#define pagealign(_x)            (alignup(_x, PAGE_SIZE))

/**
 * @brief Align a value downwards to the nearest alignment boundary.
 * @param x Value to align.
 * @param align Alignment boundary (must be non-zero).
 * @returns The largest value <= x that is a multiple of align.
 */
#define aligndown(x, align)      ((x) / (align) * (align))

/**
 * @brief Check whether a value is aligned to a boundary.
 * @param x Value to check.
 * @param align Alignment boundary.
 * @returns True if x is a multiple of align.
 */
#define isaligned(x, align)      ((x) % (align) == 0)

/**
 * @brief Hint to the compiler that the condition
 * is likely to be true at runtime.
 */
#define likely(x)                __builtin_expect(!!(x), 1)

/**
 * @brief Hint to the compiler that the condition is likely to be
 * false at runtime.
 * @sa likely
 */
#define unlikely(x)              __builtin_expect(!!(x), 0)

/** @brief Shorthand macro to mark a function parameter as intentionally unused without generating a
 * warning.  */
#define UnusedParameter(p)       ((void)p)

/**
 * @brief Preconfigured address of the global framebuffer, usually placed 8MBs before cutoff from
 * the address space.
 * @warning This approach is not scalable, and will be replaced in the future. Eg. A high resolution
 * (1080p) framebuffer will crash the kernel.
 */
#define FRAMEBUFFER_ADDR         ((uintptr_t)0xFF800000)
