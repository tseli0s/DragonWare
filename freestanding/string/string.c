/**********************************************************************
 * FILE: string.c
 * PURPOSE: String functions for DragonWare
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>
#include <limits.h>
#include <macros.h>
#include <stdarg.h>

#include "kstring.h"

/* Default number of Bytes allocated per buffer, 32 suffices for all numbers*/
#define NUMBUF (32)

enum Base { BASE8 = 8, BASE10 = 10, BASE16 = 16 };

Size strnlen(const char *s, Size maxlen) {
        Size i = 0;
        while (i < maxlen) {
                if (s[i] == '\0') break;
                i++;
        }
        return i;
}

char *strcpy(char *dest, const char *src) {
        char *dest_ptr = dest;
        while (*src != '\0') {
                *dest_ptr = *src;
                dest_ptr++;
                src++;
        }
        *dest_ptr = '\0';
        return dest;
}

char *strncpy(char *dest, const char *src, Size n) {
        char *dest_ptr = dest;
        Size  i        = 0;
        while (i < n && src[i] != '\0') {
                dest_ptr[i] = src[i];
                i++;
        }
        while (i < n) {
                dest_ptr[i] = 0x0;
                i++;
        }
        return dest;
}

char *strupr(char *str) {
        if (!str) return NullPointer;
        for (char *p = str; *p; ++p) *p = (char)toupper((unsigned char)*p);

        return str;
}

const char *strstr(const char *haystack, const char *needle) {
        if (!*needle) return haystack;

        for (; *haystack; haystack++) {
                const char *h = haystack;
                const char *n = needle;

                while (*h && *n && *h == *n) {
                        h++;
                        n++;
                }

                if (!*n) return haystack;
        }

        return NullPointer;
}

char *strchr(const char *str, int c) {
        if (!str) return NullPointer;
        Size maxcmp = strlen(str) + 1;
        while (maxcmp--)
                if (str[maxcmp] == c) return (char *)&str[maxcmp];
        return NullPointer;
}

int strncmp(const char *str1, const char *str2, Size maxlen) {
        if (!str1 || !str2 || !maxlen) return 0; /* idk what to tell you bro */

        Size start = 0;
        while (*str1 && *str2 && start <= maxlen) {
                if (*str1 != *str2) {
                        if (*str1 > *str2)
                                return *str2 - *str1;
                        else if (*str2 > *str1)
                                return *str1 - *str2;
                }
                start++;
                str1++;
                str2++;
        }
        return 0;
}

void itoa(unsigned int value, char *str, int base) {
        if (base < 2 || base > BASE16) {
                str[0] = '\0';
                return;
        }

        char buf[NUMBUF] = {0};
        int  i           = 0;

        if (value == 0) {
                str[0] = '0';
                str[1] = '\0';
                return;
        }

        while (value) {
                unsigned int digit = value % (unsigned int)base;
                buf[i++]           = (char)((digit < 10) ? ('0' + digit) : ('a' + digit - 10));
                value /= (unsigned int)base;
        }

        int j = 0;
        while (i > 0) {
                str[j++] = buf[--i];
        }
        str[j] = '\0';
}

void itoa_z(u64 value, char *str, int base) {
        if (base < 2 || base > BASE16) {
                str[0] = '\0';
                return;
        }

        /* 64 bit numbers need a little more memory because they're so much larger */
        char buf[65] = {0};
        Size i       = 0;

        if (value == 0) {
                str[0] = '0';
                str[1] = '\0';
                return;
        }

        while (value > 0) {
                unsigned int digit = value % (unsigned int)base;
                buf[i++]           = (char)((digit < 10) ? ('0' + digit) : ('a' + digit - 10));
                value /= (unsigned int)base;
        }

        Size j = 0;
        while (i > 0) str[j++] = buf[--i];
        str[j] = '\0';
}

const char *StatusCodeToString(const Status code) {
        switch (code) {
                case STATUS_OK:
                        return "Good Status";
                case STATUS_BAD:
                        return "Bad Status";
                case STATUS_BAD_ARGUMENT:
                        return "Invalid argument supplied";
                case STATUS_RETRY:
                        return "Operation must be repeated";
                case STATUS_OUT_OF_MEMORY:
                        return "Kernel has ran out of memory";
                case STATUS_UNSUPPORTED:
                        return "Unsupported request";
                case STATUS_TIMEOUT:
                        return "Resource not available at this time";
                case STATUS_NO_ENDPOINT:
                        return "No endpoint for message";
                case STATUS_BAD_SYSCALL:
                        return "Bad system call number given to kernel";
                default:
                        return "Bad Code";
        }
        return "Bad Code";
}

/* Yeah I know the names here look too old school and Unixy they're just utility functions that will
 * be replaced later. */
static inline void bputc(char *buf, size_t size, size_t *idx, char c) {
        if (size > 0 && *idx < size - 1) buf[*idx] = c;
        (*idx)++;
}

static inline void bputs(char *buf, size_t size, size_t *idx, const char *s) {
        if (!s) s = "(null)";
        while (*s) bputc(buf, size, idx, *s++);
}

static void bputud(char *buf, size_t size, size_t *idx, u64 v, unsigned int base, Bool upper) {
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
int vsnprintf(char *restrict buf, Size maxsize, const char *restrict fmt, va_list ap) {
        Size idx = 0;

        for (; *fmt; fmt++) {
                if (*fmt != '%') {
                        bputc(buf, maxsize, &idx, *fmt);
                        continue;
                }

                fmt++;
                if (!*fmt) break;

                switch (*fmt) {
                        case '%':
                                bputc(buf, maxsize, &idx, '%');
                                break;

                        case 'c': {
                                int c = va_arg(ap, int);
                                bputc(buf, maxsize, &idx, (char)c);
                                break;
                        }

                        case 's': {
                                const char *s = va_arg(ap, const char *);
                                bputs(buf, maxsize, &idx, s);
                                break;
                        }

                        case 'd':
                        case 'i': {
                                int v = va_arg(ap, int);
                                if (v < 0) {
                                        bputc(buf, maxsize, &idx, '-');
                                        u64 uv = (u64)(-(i64)v);
                                        bputud(buf, maxsize, &idx, uv, 10, false);
                                } else {
                                        bputud(buf, maxsize, &idx, (u64)(unsigned)v, 10, false);
                                }
                                break;
                        }

                        case 'u': {
                                unsigned int v = va_arg(ap, unsigned int);
                                bputud(buf, maxsize, &idx, (u64)v, 10, false);
                                break;
                        }

                        case 'x': {
                                unsigned int v = va_arg(ap, unsigned int);
                                bputud(buf, maxsize, &idx, (u64)v, 16, false);
                                break;
                        }

                        case 'X': {
                                unsigned int v = va_arg(ap, unsigned int);
                                bputud(buf, maxsize, &idx, (u64)v, 16, true);
                                break;
                        }

                        case 'p': {
                                void *v = va_arg(ap, void *);
                                bputs(buf, maxsize, &idx, "0x");
                                bputud(buf, maxsize, &idx, (uintptr_t)v, 16, false);
                                break;
                        }

                        /* Long pointers. Used, eg, in PAE setups, where addresses may be above
                         * UINT32_MAX. */
                        case 'r': {
                                u64 v = va_arg(ap, u64);
                                bputs(buf, maxsize, &idx, "0x");
                                bputud(buf, maxsize, &idx, v, 16, false);
                                break;
                        }
                        case 'f':
                                /* TODO: Floats*/
                                break;
                        default:
                                bputc(buf, maxsize, &idx, '%');
                                bputc(buf, maxsize, &idx, *fmt);
                                break;
                }
        }

        if (maxsize > 0) {
                if (idx >= maxsize)
                        buf[maxsize - 1] = '\0';
                else
                        buf[idx] = '\0';
        }

        if (idx > (Size)INT_MAX) return INT_MAX;
        return (int)idx;
}

int snprintf(char *restrict str, Size maxsize, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(str, maxsize, fmt, ap);
        va_end(ap);
        return n;
}

Bool StringHasPrefix(const char *str, const char *prefix, Size maxcmp) {
        Bool cmp = true;
        if (!maxcmp) {
                Size len = min(strlen(str), strlen(prefix));
                for (Size i = 0; i < len; i++) {
                        if (*str == '\0' || *prefix == '\0') break;
                        if (str[i] != prefix[i]) {
                                cmp = false;
                                break;
                        }
                }
        } else {
                for (Size i = 0; i < maxcmp; i++) {
                        if (*str == '\0' || *prefix == '\0') break;
                        if (str[i] != prefix[i]) {
                                cmp = false;
                                break;
                        }
                }
        }
        return cmp;
}
