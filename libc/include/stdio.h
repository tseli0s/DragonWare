/**********************************************************************
 * FILE: stdio.h
 * PURPOSE: stdio.h libc header definitions
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define __STDC_VERSION_STDIO_H__ 202311L
#define EOF                      (-1)
#define FOPEN_MAX                (28)

#include "stdarg.h"
#include "stddef.h"

typedef struct __file_impl FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

/**
 * @brief Produces output in the standard output (See @ref stdout), formatted according to @p fmt if
 * there are formatting modifiers in it.
 * @param[in] fmt The text to write to @ref stdout. If this C string contains formatting modifiers
 * (eg. @b %d ) then the output is formatted accordingly. Arguments are passed after @p fmt as
 * variadic arguments.
 * @returns The total amount of characters written in @ref stdout on success. If an error occurs
 * during write, @ref EOF is returned instead.
 * @bug On DragonWare, resulting (formatted or not) strings that exceed the size of a single IPC
 * message payload minus two bytes will lead to the output string being truncated, due to existing
 * limitations with the console implementation.
 */
[[gnu::format(printf, 1, 2)]]
int printf(const char *restrict fmt, ...);

int putchar(int c);

/* Note: DragonWare doesn't have file descriptors or a notion of stdout/stderr etc, fd will be
 * ignored. It's for compatibility reasons only. */
[[gnu::format(printf, 2, 3)]]
int fprintf(FILE *restrict stream, const char *fmt, ...);

[[gnu::format(printf, 2, 3)]]
int sprintf(char *restrict str, const char *restrict fmt, ...);

int vprintf(const char *restrict fmt, va_list args);

[[gnu::format(printf, 3, 4)]]
int snprintf(char *restrict str, size_t maxsize, const char *restrict fmt, ...);

int vsprintf(char *restrict str, const char *restrict fmt, va_list args);

int vsnprintf(char *restrict str, size_t maxsize, const char *restrict fmt, va_list args);

/* TODO as well */
int puts(const char *str);

int fputc(int c, FILE *stream);
