/**********************************************************************
 * FILE: string.h
 * PURPOSE: string.h libc header definitions
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "dlibc_common.h"
#include "stddef.h"

DLC_BEGIN_DECLS

void  *memcpy(void *restrict dest, const void *restrict src, size_t size);
void  *memmove(void *dest, const void *src, size_t size);
void  *memset(void *dest, int value, size_t size);
int    memcmp(const void *p1, const void *p2, size_t n);
char  *strcpy(char *dest, const char *src);
char  *strncpy(char *dest, const char *src, size_t size);
char  *strcat(char *dest, const char *src);
char  *strncat(char *dest, const char *src, size_t n);
int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);
char  *strchr(const char *s, int c);
size_t strlen(const char *s);
char  *strdup(const char *s);

DLC_END_DECLS
