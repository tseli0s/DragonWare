/**********************************************************************
 * FILE: stdio.c
 * PURPOSE: Standard I/O libc function implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "stdio.h"

#include "errno.h"
#include "ipc86.h"
#include "kernelapi.h"
#include "kerneltypes.h"
#include "limits.h"
#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"
#include "string.h"
#include "syscalls/syscall86.h"
#include "vgacons/protocol.h"

/* Temporary limitation. We can only print what can be contained in one message, and further
 * characters must be printed using more messages. Plus obviously the NULL terminator. */
#define MAX_PRINTF_SIZE (MESSAGE_BUFFER_SIZE - 1)

FILE *stdin  = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

extern int          __dlibc_console_handle;
extern volatile int errno;

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

static int __send_console_write_string_request(const char *consolebuf) {
        size_t buflen = strlen(consolebuf);
        if (buflen > MAX_PRINTF_SIZE) return -1;

        Message m;
        m.header.payload_length = buflen + 1;
        m.header.protocol       = VGACONS_PROTOCOL_V0;
        m.header.type           = VGACONS_REQUEST_STRING_DRAW;
        m.header.reply_handle   = -1;

        memcpy(m.payload.raw, consolebuf, buflen + 1);
        Status send = STATUS_BAD;
        do {
                send = (Status)__make_syscall_ia32_3param_reti32(
                        SYSCALL_SEND, (uint32_t)__dlibc_console_handle, (uint32_t)&m,
                        sizeof(m.header) + (size_t)m.header.payload_length);

                /* waiting for the server to come online */
                if (send == STATUS_RETRY || send == STATUS_NO_ENDPOINT)
                        __make_syscall_ia32_0param(SYSCALL_YIELD);
        } while (send != STATUS_OK);
        return 0;
}

/* TODO: use errno values here */
int printf(const char *fmt, ...) {
        if (__dlibc_console_handle < 0) {
                errno = EIO;
                return EOF;
        }
        /* TODO: Handle larger than 254 character strings (Possibly by
        dispatching multiple messages to the server)*/
        char    str[MAX_PRINTF_SIZE];
        va_list ap;
        va_start(ap, fmt);
        int result = vsnprintf(str, sizeof(str), fmt, ap);
        va_end(ap);

        if (result < 0) {
                errno = EINVAL;
                return EOF;
        }

        if (__send_console_write_string_request(str) < 0) {
                errno = ENOTCONN;
                return EOF;
        }
        return result;
}

int vprintf(const char *restrict format, va_list arg) {
        char buf[MAX_PRINTF_SIZE];
        int  result = vsnprintf(buf, MAX_PRINTF_SIZE, format, arg);

        if (__send_console_write_string_request(buf) < 0) return EOF;
        return result;
}

int fprintf(FILE *restrict stream, const char *fmt, ...) {
        errno = ENOENT; /* Couldn't come up with something better this'll do */
        return -1;
}

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
                                void *v = va_arg(args, void *);
                                bputs(str, maxsize, &idx, "0x");
                                bputud(str, maxsize, &idx, (uintptr_t)v, 16, false);
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

int putchar(int c) {
        unsigned char uc = (unsigned char)c;
        Message       m;
        m.header.protocol       = VGACONS_PROTOCOL_V0;
        m.header.type           = VGACONS_REQUEST_CHAR_DRAW;
        m.header.reply_handle   = -1;
        m.header.payload_length = 1;

        /* Per protocol, bytes 1-3 must be zero, and byte 0 will contain the character, but it's
         * faster to actually zero first and then assign (because of alignment stuff) */
        memset(m.payload.raw, 0, 4);
        m.payload.raw[0] = uc;

        Status send = STATUS_BAD;
        do {
                send = (Status)__make_syscall_ia32_3param_reti32(
                        SYSCALL_SEND, (uint32_t)__dlibc_console_handle, (uint32_t)&m,
                        sizeof(m.header) + 1);
                __make_syscall_ia32_0param(SYSCALL_YIELD);
        } while (send == STATUS_RETRY || send == STATUS_NO_ENDPOINT);

        if (send != STATUS_OK) {
                errno = ECOMM;

                /* we should return EOF if the character wasn't printed */
                return EOF;
        }

        return c;
}
int puts(const char *str) { return printf("%s\n", str); }

int fputc(int c, FILE *stream) {
        errno = ENOENT; /* see comment in fprintf */
        return EOF;
}
