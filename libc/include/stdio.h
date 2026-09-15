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

/**
 * @brief Writes a single character @p c into the standard output stream @ref stdout.
 * @param c The character to write. Must be a @b char that is automatically promoted to an int
 * according to the C standard conventions. The character is internally converted into an unsigned
 * char.
 * @returns @p c if the write succeeded. On failure, @ref EOF is returned.
 */
int putchar(int c);

/**
 * @warning This function is unimplemented. Using it will only cause runtime errors. See @ref printf
 * instead.
 * @param[in] stream Unused paramter.
 * @param[in] fmt Unused parameter.
 * @returns @b fprintf always returns EOF, and sets @ref errno to ENOENT. DragonWare's C library
 * does not support file access at the moment.
 */
[[gnu::format(printf, 2, 3)]]
int fprintf(FILE *restrict stream, const char *fmt, ...);

/**
 * @brief Formats a string according to the formatting specifiers in @p fmt and copies the resulting
 * string into @p str.
 * @param[out] str Address to store the resulting string produced by this function. The buffer
 * pointed to by @p str must be sufficiently large to store the formatted string.
 * @param[in] fmt C string that may contain formatting specifiers, according to which the output
 * string is going to be formatted.
 * @returns The total amount of characters written into the buffer pointed to by @p str, or a
 * negative integer on failure.
 */
[[gnu::format(printf, 2, 3)]]
int sprintf(char *restrict str, const char *restrict fmt, ...);

/**
 * @brief Writes the C string @p fmt to @ref stdout, formatting the input string @p fmt according to
 * the same formatting rules as @ref printf, but using the elements in the variable argument
 * list identified by @p args instead of additional function arguments.
 * @param[in] fmt C string that may contain formatting specifiers.
 * @param[in] args A value identifying a variable arguments list initialized with @ref va_start.
 * @sa printf
 * @bug Just like @ref printf, on DragonWare, resulting (formatted or not) strings that exceed the
 * size of a single IPC message payload minus two bytes will lead to the output string being
 * truncated, due to existing limitations with the console implementation.
 * @returns The total amount of characters written to @ref stdout, or a negative integer on failure.
 */
[[gnu::format(printf, 1, 0)]]
int vprintf(const char *restrict fmt, va_list args);

/**
 * @brief Takes a C string @p fmt that may contain formatting specifiers and, after formatting the
 * arguments appropriately, stores the resulting formatted string into @p str, writing no more than
 * @p maxsize bytes.
 * @param[in] str A pointer to a buffer at least @p maxsize bytes
 * @param[in] maxsize Amount of bytes to write. If the resulting string's length formatted by this
 * function is less than @p maxsize, then the rest of the memory is overwritten with zeroes in @p
 * str.
 * @param[in] fmt A C string that may contain formatting specifiers in a similar format to @ref
 * printf.
 * @returns The amount of characters that would have been written (Excluding the null terminator
 * byte), had @p maxsize been sufficiently large. If this function's return value is less than @p
 * maxsize then no truncation occurred, otherwise truncation did occur. If an encoding error
 * occurred, then @ref EOF is returned instead.
 */
[[gnu::format(printf, 3, 4)]]
int snprintf(char *restrict str, size_t maxsize, const char *restrict fmt, ...);

/**
 * @brief Writes the C string @p fmt to @p str, formatting the input string @p fmt according to
 * the same formatting rules as @ref printf, but using the elements in the variable argument
 * list identified by @p args instead of additional function arguments.
 * @param[in] str A pointer to a buffer large enough to store the resulting string.
 * @param[in] fmt C string that may contain formatting specifiers.
 * @param[in] args A value identifying a variable arguments list initialized with @ref va_start.
 * @returns The total amount of characters written to @p str, or a negative integer on failure.
 */
int vsprintf(char *restrict str, const char *restrict fmt, va_list args);

/**
 * @brief Writes the C string @p fmt to @p str, formatting the input string @p fmt according to
 * the same formatting rules as @ref printf, but using the elements in the variable argument
 * list identified by @p args instead of additional function arguments, writing no more than @p
 * maxsize bytes into @p str.
 * @param[out] str A pointer to a buffer to copy the resulting string to.
 * @param[in] maxsize Maximum amount of bytes to write to @p str. The resulting string, if the
 * length of which is greater or equal to @p maxsize, will not exceed @p maxsize - 1 characters in
 * length, with the last character reserved for the null terminator.
 * @param[in] fmt A C string that may contain formatting specifiers.
 * @param[in] args A value identifying a variable arguments list initialized with @ref va_start.
 * @returns The amount of characters that would have been written (Excluding the null terminator
 * byte), had @p maxsize been sufficiently large. If this function's return value is less than @p
 * maxsize then no truncation occurred, otherwise truncation did occur. If an encoding error
 * occurred, then @ref EOF is returned instead.
 */
int vsnprintf(char *restrict str, size_t maxsize, const char *restrict fmt, va_list args);

/**
 * @brief Writes a C string @p str into @ref stdout and automatically appends a newline byte @code
 * \n @endcode afterwards. The null terminator byte is not written.
 * @param[in] str The C string to write to @ref stdout.
 * @returns @ref EOF is a write error occurs. On success, a nonnegative integer is returned.
 */
int puts(const char *str);

/**
 * @warning This function is unimplemented. Using it will only cause runtime errors. See @ref putc
 * instead.
 * @param[in] c Unused parameter.
 * @param[in] stream Unused parameter.
 * @returns @b fputc always returns EOF, and sets @ref errno to ENOENT. DragonWare's C library
 * does not support file access at the moment.
 */
int fputc(int c, FILE *stream);
