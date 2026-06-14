/**********************************************************************
 * FILE: string.c
 * PURPOSE: String and memory manipulation functions libc implementation
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "string.h"

#include "stdlib.h"

void *memcpy(void *restrict dest, const void *restrict src, size_t size) {
        if (!size) return dest;

        char       *d = dest;
        const char *s = src;
        while (size--) {
                d[size] = s[size];
        }
        return dest;
}

void *memmove(void *dest, const void *src, size_t size) {
        if (!size || dest == src) return dest;

        char       *d = dest;
        const char *s = src;

        if (d < s) {
                while (size--) *d++ = *s++;
        } else {
                d += size;
                s += size;
                while (size--) *--d = *--s;
        }
        return dest;
}

void *memset(void *dest, int value, size_t size) {
        uint8_t *d = dest;
        while (size--) *d++ = (uint8_t)value;
        return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
        const unsigned char *a = s1;
        const unsigned char *b = s2;

        for (; n; n--, a++, b++) {
                if (*a != *b) return *a - *b;
        }
        return 0;
}

char *strcpy(char *dest, const char *src) {
        char *ret = dest;
        while ((*dest++ = *src++)); /* shut up clang*/

        return ret;
}

char *strncpy(char *dest, const char *src, size_t n) {
        char *ret = dest;
        while (n && *src) {
                *dest++ = *src++;
                n--;
        }
        while (n--) {
                *dest++ = '\0';
        }
        return ret;
}

char *strcat(char *dest, const char *src) {
        char *ret = dest;
        while (*dest) dest++;
        while ((*dest++ = *src++));

        return ret;
}

char *strncat(char *dest, const char *src, size_t n) {
        char *ret = dest;
        while (*dest) dest++;
        while (n && *src) {
                *dest++ = *src++;
                n--;
        }
        *dest = '\0';
        return ret;
}

int strcmp(const char *s1, const char *s2) {
        while (*s1 && (*s1 == *s2)) {
                s1++;
                s2++;
        }
        return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
        while (n && *s1 && (*s1 == *s2)) {
                s1++;
                s2++;
                n--;
        }

        if (n == 0) return 0;

        return (unsigned char)*s1 - (unsigned char)*s2;
}
/* TODO: strcasecmp ... */

char *strchr(const char *s, int c) {
        char ch = (char)c;
        while (*s) {
                if (*s == ch) return (char *)s;
                s++;
        }
        return ch == '\0' ? (char *)s : NULL;
}

size_t strlen(const char *s) {
        const char *p = s;
        while (*p) p++;
        return (size_t)(p - s);
}

/*
char *strdup(const char *s) {
        const size_t len = strlen(s);
        char        *str = malloc(len);
        if (!str) return NULL;

        memcpy(str, s, len);
        str[len] = '\0';

        return str;
}
*/
