/**********************************************************************
 * FILE: stdlib.c
 * PURPOSE: Implementation for functions found in stdlib.h libc standard header
 * PROJECT: DragonWare C Library
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "stdlib.h"

#include "kernelapi.h"
#include "stddef.h"

int isspace(int c) {
        return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r');
}

int isdigit(int c) { return (c >= '0' && c <= '9'); }

int atoi(const char *str) {
        int sign   = 1;
        int result = 0;

        while (isspace(*str)) str++;

        if (*str == '+' || *str == '-') {
                if (*str == '-') sign = -1;
                str++;
        }

        while (isdigit(*str)) {
                result = result * 10 + (*str - '0');
                str++;
        }

        return sign * result;
}

long atol(const char *str) {
        int  sign   = 1;
        long result = 0;

        while (isspace(*str)) str++;

        if (*str == '+' || *str == '-') {
                if (*str == '-') sign = -1;
                str++;
        }

        while (isdigit(*str)) {
                result = result * 10 + (*str - '0');
                str++;
        }

        return sign * result;
}

long long atoll(const char *str) {
        int       sign   = 1;
        long long result = 0;

        while (isspace(*str)) str++;

        if (*str == '+' || *str == '-') {
                if (*str == '-') sign = -1;
                str++;
        }

        while (isdigit(*str)) {
                result = result * 10 + (*str - '0');
                str++;
        }

        return sign * result;
}

double atof(const char *str) {
        double result   = 0.0;
        double frac     = 0.0;
        double div      = 1.0;
        int    sign     = 1;
        int    exp_sign = 1;
        int    exp      = 0;

        while (isspace(*str)) str++;

        if (*str == '+' || *str == '-') {
                if (*str == '-') sign = -1;
                str++;
        }

        while (isdigit(*str)) {
                result = result * 10.0 + (*str - '0');
                str++;
        }

        if (*str == '.') {
                str++;
                while (isdigit(*str)) {
                        frac = frac * 10.0 + (*str - '0');
                        div *= 10.0;
                        str++;
                }
        }

        result += frac / div;

        if (*str == 'e' || *str == 'E') {
                str++;
                if (*str == '+' || *str == '-') {
                        if (*str == '-') exp_sign = -1;
                        str++;
                }

                while (isdigit(*str)) {
                        exp = exp * 10 + (*str - '0');
                        str++;
                }

                while (exp-- > 0) {
                        if (exp_sign > 0)
                                result *= 10.0;
                        else
                                result /= 10.0;
                }
        }
        return sign * result;
}

/* TODO: Random number generation functions */

void exit(int status) {
        (void)status; /* DragonWare doesn't support that, yet. */
        _DWExit();
#ifdef __GNUC__
        __builtin_unreachable();
#endif /* __GNUC__ */
}

char *getenv(const char *name) {
        (void)name;
        /* Environment variables won't be a thing in DragonWare, so just return NULL. */
        return NULL;
}

int setenv(const char *name, const char *value, int replace) {
        (void)name;
        (void)value;
        (void)replace;
        return -1;
}

int unsetenv(const char *name) {
        (void)name;
        return -1;
}

int abs(int x) { return (x >= 0) ? x : x * (-1); }

long int labs(long int x) { return (x >= 0) ? x : x * (-1); }
