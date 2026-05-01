/**********************************************************************
 * FILE: stdlib.h
 * PURPOSE: stdlib.h libc header definitions
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "dlibc_common.h"
#include "limits.h"
#include "stddef.h"

DLC_BEGIN_DECLS

#define RAND_MAX     INT32_MAX
#define EXIT_FAILURE (-1)
#define EXIT_SUCCESS (0)

/* function pointer to be ran when exit() is called */
typedef void (*exit_func_t)(void);

int isdigit(int c);
int isspace(int c);

double    atof(const char *str);
int       atoi(const char *str);
long      atol(const char *str);
long long atoll(const char *str);

long int random(void);
void     srandom(unsigned int seed);
int      rand(void);
void     srand(unsigned int seed);

void *malloc(size_t size);
void *calloc(size_t count, size_t size_each);
void *realloc(void *prev, size_t new_size);
void  free(void *ptr);

extern void abort(void);
extern int  atexit(exit_func_t func);
void        exit(int status);

char *getenv(const char *name);
int   setenv(const char *name, const char *value, int replace);
int   unsetenv(const char *name);

int system(char *cmd);

int      abs(int x);
long int labs(long int x);

int    mblen(const char *str, size_t n);
int    mbtowc(wchar_t *restrict buf, const char *restrict str, size_t n);
int    wctomb(char *str, wchar_t wchar);
size_t mbstowcs(wchar_t *restrict buf, const char *restrict str, size_t n);
size_t wcstombs(char *restrict str, const wchar_t *restrict buf, size_t n);

DLC_END_DECLS
