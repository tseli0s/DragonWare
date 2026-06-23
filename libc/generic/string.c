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
