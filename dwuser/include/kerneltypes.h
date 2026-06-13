/**********************************************************************
 * FILE: kerneltypes.h
 * PURPOSE: DragonWare kernel types reexported to userspace from the freestanding library
 * PROJECT: DragonWare User Library
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#ifndef _KERNEL_TYPES_H
#define _KERNEL_TYPES_H 1

#include "cppsupport.h"

DW_BEGIN_DECLS

#ifndef __i386__
#error Compile DragonWare with a compiler targeting i386 CPUs!
#endif

#ifdef __linux
#error You are trying to compile DragonWare as a Linux app. Thats not a good idea and probably not what you intended.
#endif

#define NullPointer nullptr /* I like the lowercase version better */

#ifdef __i386__
#include "arch/ia32/inttypes.h"
#elifdef __AMD64__
#include "arch/amd64/inttypes.h"
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

/* Why not have this too? */
typedef u64 Quad;

/* I like PascalCase. */
typedef bool Bool;
typedef u32  Size;

#define BITS_BYTE  8
#define BITS_WORD  16
#define BITS_DWORD 32
#define BITS_QUAD  64

/**
 * @brief Status codes for system calls and other kernel operations
 * @details This type defines a generic status code implementation, originally only used inside the
 * DragonWare kernel. It is mostly relevant for system calls, the only part of DragonWare's
 * microkernel exposed to userland
 */
typedef enum _Status {
        STATUS_OK            = 0x0, /** << No error */
        STATUS_BAD           = -1,  /** << Bad unspecified status */
        STATUS_OUT_OF_MEMORY = -2,  /** << Out of memory or allocation failed */
        STATUS_BAD_ARGUMENT  = -3,  /** << Bad function/system call argument provided */
        STATUS_RETRY         = -4,  /** << Operation must be repeated */
        STATUS_UNSUPPORTED   = -5,  /** << Unsupported operation */
        STATUS_NOT_FOUND     = -6,  /** << Resource not found */
        STATUS_OUT_OF_BOUNDS = -7,  /** << Attempted to read an object out of its bounds */
        STATUS_TIMEOUT       = -8,  /** << Resource did not become available in valid time frame */
        STATUS_NO_ENDPOINT   = -9,  /** << Message has no valid recipient to be sent to */
        STATUS_BAD_SYSCALL   = -10, /** << Bad or unrecognized system call number */
} Status;

#define KSUCCESS(s) ((Status)(s) == STATUS_OK)
#define KFAILED(s)  ((Status)(s) != STATUS_OK)

/**
 * @brief Logging severity levels reexported from the kernel for the _DWklog system call (#3)
 * @details Used to classify log output by importance. Higher values indicate
 * more severe conditions.
 */
typedef enum {
        LOG_DEBUG   = 0, /**< Debug-level messages (verbose, for development). */
        LOG_INFO    = 1, /**< Informational messages. */
        LOG_WARNING = 2, /**< Warning messages (non-fatal issues). */
        LOG_ERROR   = 3, /**< Error messages (fatal or critical issues). */
} LogLevel;

DW_END_DECLS

#endif /* _KERNEL_TYPES_H */
