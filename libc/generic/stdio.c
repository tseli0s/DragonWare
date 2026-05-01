/**********************************************************************
 * FILE: stdio.c
 * PURPOSE: Standard I/O libc function implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "stdio.h"

#include "limits.h"
#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"

FILE *stdin  = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

static inline void bputc(char *buf, size_t size, size_t *idx, char c) {
        if (size > 0 && *idx < size - 1) buf[*idx] = c;
        (*idx)++;
}

static inline void bputs(char *buf, size_t size, size_t *idx, const char *s) {
        if (!s) s = "(null)";
        while (*s) bputc(buf, size, idx, *s++);
}

static void bputud(char *buf, size_t size, size_t *idx, uint64_t v, unsigned int base, bool upper) {
        char tmp[66];
        int  n = 0;
        if (base < 2 || base > 16) return;
        if (v == 0) {
                tmp[n++] = '0';
        } else {
                while (v) {
                        unsigned int digit = (unsigned int)(v % base);
                        v /= base;
                        if (digit < 10)
                                tmp[n++] = (char)('0' + digit);
                        else
                                tmp[n++] = (char)(((upper ? 'A' : 'a')) + (digit - 10));
                }
        }
        while (n--) bputc(buf, size, idx, tmp[n]);
}

int printf(const char *fmt, ...) { return -1; /* TODO */ }
int fprintf(FILE *restrict stream, const char *fmt, ...) { return -1; }

int sprintf(char *restrict str, const char *restrict fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int result = vsnprintf(str, INT32_MAX, fmt, args);
        va_end(args);
        return result;
}

int snprintf(char *restrict str, size_t maxsize, const char *restrict fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int result = vsnprintf(str, maxsize, fmt, args);
        va_end(args);
        return result;
}

int vsprintf(char *restrict str, const char *restrict fmt, va_list args) {
        /* lol */
        return vsnprintf(str, INT32_MAX, fmt, args);
}

/* ported from the freestanding library */
int vsnprintf(char *restrict str, size_t maxsize, const char *restrict fmt, va_list args) {
        size_t idx = 0;

        for (; *fmt; fmt++) {
                if (*fmt != '%') {
                        bputc(str, maxsize, &idx, *fmt);
                        continue;
                }

                fmt++;
                if (!*fmt) break;

                switch (*fmt) {
                        case '%':
                                bputc(str, maxsize, &idx, '%');
                                break;

                        case 'c': {
                                int c = va_arg(args, int);
                                bputc(str, maxsize, &idx, (char)c);
                                break;
                        }

                        case 's': {
                                const char *s = va_arg(args, const char *);
                                bputs(str, maxsize, &idx, s);
                                break;
                        }

                        case 'd':
                        case 'i': {
                                int v = va_arg(args, int);
                                if (v < 0) {
                                        bputc(str, maxsize, &idx, '-');
                                        uint64_t uv = (uint64_t)(-(int64_t)v);
                                        bputud(str, maxsize, &idx, uv, 10, false);
                                } else {
                                        bputud(str, maxsize, &idx, (uint64_t)(unsigned)v, 10,
                                               false);
                                }
                                break;
                        }

                        case 'u': {
                                unsigned int v = va_arg(args, unsigned int);
                                bputud(str, maxsize, &idx, (uint64_t)v, 10, false);
                                break;
                        }

                        case 'x': {
                                unsigned int v = va_arg(args, unsigned int);
                                bputud(str, maxsize, &idx, (uint64_t)v, 16, false);
                                break;
                        }

                        case 'X': {
                                unsigned int v = va_arg(args, unsigned int);
                                bputud(str, maxsize, &idx, (uint64_t)v, 16, true);
                                break;
                        }

                        case 'p': {
                                uint32_t v = va_arg(args, uint32_t);
                                bputs(str, maxsize, &idx, "0x");
                                bputud(str, maxsize, &idx, v, 16, false);
                                break;
                        }
                        case 'f':
                                /* TODO: Floats (this also goes for the freestanding implementation
                                 * btw)*/
                                break;
                        default:
                                bputc(str, maxsize, &idx, '%');
                                bputc(str, maxsize, &idx, *fmt);
                                break;
                }
        }

        if (maxsize > 0) {
                if (idx >= maxsize)
                        str[maxsize - 1] = '\0';
                else
                        str[idx] = '\0';
        }

        if (idx > (size_t)INT_MAX) return INT_MAX;
        return (int)idx;
}

int puts(const char *str) { return printf("%s\n", str); }
int fputc(int c, FILE *stream) { return c; }
