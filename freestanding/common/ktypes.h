/**********************************************************************
 * FILE: ktypes.h
 * PURPOSE: Common type definitions for DragonWare
 * PROJECT: DragonWare Freestanding Library
 * DATE: 2025-08
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef __i386__
#error Compile DragonWare with a compiler targeting i386 CPUs!
#endif

#ifdef __linux
#error You are trying to compile DragonWare as a Linux app. Thats not a good idea and probably not what you intended.
#endif

#ifndef NULL
#define NULL nullptr
#endif
#define NullPointer NULL /* I like the lowercase version better */

#ifdef __i386__
#include "../arch/ia32/inttypes.h"
#elifdef __AMD64__ /* or whatever the 64 bit version is it's not like it'll work */
#include "../arch/amd64/inttypes.h"
#endif /* __i386__ */

/*
 * It's not necessary, but I like the byte definition better. PS: I saw a
 * massive argument online about whether a byte is always supposed to be eight
 * bits or the width of a char (which is almost always eight bits but not
 * always). I don't care, I'll settle with the unsigned char crowd.
 *
 * Btw, don't use this for strings. Just use char. */
typedef u8 Byte;

/*
 * A word is supposed to be the size of the pointer width of the target machine
 * (so, in IA32, 4 bytes). However x86 is being an oddball here, whatever, I guess
 * u16 is our word.
 */
typedef u16 Word;

/*
 * Yeah I am gonna call it a dword for the reasons mentioned above
 * (But I won't use it, so who cares)
 */
typedef u32 DoubleWord;

/* I like PascalCase. */
typedef bool   Bool;
typedef size_t Size;

#define BITS_BYTE  8
#define BITS_WORD  16
#define BITS_DWORD 32
#define BITS_QUAD  64

/**
 * @brief Kernel-wide status codes for function returns.
 * * Values are structured so that:
 * - 0: Success
 * - Negative: Error conditions
 */
typedef enum _Status {
        STATUS_OK            = 0x0,
        STATUS_BAD           = -1,
        STATUS_OUT_OF_MEMORY = -2,
        STATUS_BAD_ARGUMENT  = -3,
        STATUS_RETRY         = -4,
        STATUS_UNSUPPORTED   = -5,
        STATUS_NOT_FOUND     = -6,
        STATUS_OUT_OF_BOUNDS = -7,
        STATUS_TIMEOUT       = -8,
        STATUS_NO_ENDPOINT   = -9,
        STATUS_BAD_SYSCALL   = -10,
        STATUS_MSGQUEUE_FULL = -11,
} Status;

/**
 * @brief Converts a @ref Status into a human-readable ASCII string.
 * @param code The @ref Status code to convert.
 * @returns An ASCII string that explains the error code in a human readable way.
 * @note Invalid codes will simply return a "Bad Code" string.
 * @warning Do NOT modify the returned string. It is intended to be read only and it is embedded
 * inside the kernel binary. Copy it instead if necessary using @ref strdup
 */
const char *StatusCodeToString(const Status code);
