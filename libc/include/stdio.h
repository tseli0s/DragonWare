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

int printf(const char *fmt, ...);

int putchar(int c);

/* Note: DragonWare doesn't have file descriptors or a notion of stdout/stderr etc, fd will be
 * ignored. It's for compatibility reasons only. */
int fprintf(FILE *restrict stream, const char *fmt, ...);

int sprintf(char *restrict str, const char *restrict fmt, ...);
int vprintf(const char *restrict fmt, va_list args);
int snprintf(char *restrict str, size_t maxsize, const char *restrict fmt, ...);
int vsprintf(char *restrict str, const char *restrict fmt, va_list args);
int vsnprintf(char *restrict str, size_t maxsize, const char *restrict fmt, va_list args);

/* TODO as well */
int puts(const char *str);

int fputc(int c, FILE *stream);
