/**********************************************************************
 * FILE: kstring.h
 * PURPOSE: String functions for DragonWare
 * PROJECT: DragonWare Freestanding Library
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <stdarg.h>

/**
 * @brief Computes the length of a null-terminated string.
 * @param s Pointer to the input string.
 * @returns Number of characters before the terminating null Byte.
 */
Size strlen(const char *s);

/**
 * @brief Computes the length of a string @p s but stops reading after @p maxlen
 * @param[in] s The string to compute the length of.
 * @param maxlen Maximum amount of bytes to search for the null termination byte before stopping.
 * @return The length of the string if it is null-terminated, or @p maxlen if the null terminator
 * couldn't be found.
 */
Size strnlen(const char *s, Size maxlen);

/**
 * @brief Copy a null-terminated string.
 * @param[out] dest Destination buffer.
 * @param[in] src Source string.
 * @returns Pointer to the destination buffer.
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief Copy a string with a size limit.
 * @param[out] dest Destination buffer.
 * @param[in] src Source string.
 * @param[in] size Maximum number of Bytes to copy.
 * @returns Pointer to the destination buffer.
 * @note The destination is always null-terminated if size > 0.
 */
char *strncpy(char *dest, const char *src, Size size);

/**
 * @brief Converts a string to uppercase and returns it.
 * @param[in] str The string to convert.
 * @returns @p str with all the characters making up the string converted to uppercase. Note that no
 * copies are made, strupr modifies @p str in place and returns it.
 */
char *strupr(char *str);

/**
 * @brief Compare two null-terminated strings.
 * @param[in] s1 First string.
 * @param[in] s2 Second string.
 * @returns 0 if equal, a negative value if s1 < s2, or a positive value if s1 > s2.
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Compare no more than @p maxlen bytes of @p str1 against @p str2 and return the result.
 * @param[in] str1 String to compare with
 * @param[in] str2 String to compare against
 * @param[in] maxlen How many bytes to compare before stopping
 * @return 0 if the two strings are equal, a negative value if s1 < s2, and a positive value if s1 >
 * s2.
 */
int strncmp(const char *str1, const char *str2, Size maxlen);

/**
 * @brief Find the first occurrence of a substring.
 * @param[in] haystack String to search within.
 * @param[in] needle Substring to search for.
 * @returns Pointer to the first occurrence of @p needle in @p haystack, or NullPointer if not
 * found.
 */
const char *strstr(const char *haystack, const char *needle);

/**
 * @brief Find the first occurence of (char) @p c inside @p str and return a pointer to it.
 * @param[in] str The string to search within
 * @param[in] c The character to look for, automatically converted into a char
 * @note The terminating \0 NULL byte is considered part of the string, so if c is \0 this will
 * return a pointer to the final byte of the string.
 * @returns A pointer within @p str containing this character, or NullPointer if it couldn't be
 * found.
 */
char *strchr(const char *str, int c);

/**
 * @brief Convert an unsigned integer to a string.
 * @param[in] value Value to convert.
 * @param[out] str Output buffer.
 * @param[in] base Numeric base (e.g., 10 for decimal, 16 for hexadecimal).
 */
void itoa(unsigned int value, char *str, int base);

/**
 * @brief Convert a 64-bit unsigned integer to a string.
 * @param[in] value Value to convert.
 * @param[out] str Output buffer.
 * @param[in] base Numeric base (e.g., 10 for decimal, 16 for hexadecimal).
 * @note This is equivalent to @ref itoa but supports 64-bit values.
 */
void itoa_z(u64 value, char *str, int base);

/**
 * @brief Duplicate a string.
 * @param[in] src Source string.
 * @returns Pointer to a newly allocated copy of the string, or NULL on failure.
 */
char *kstrdup(const char *src);

/**
 * @brief This function is similar to ‘snprintf’ except that, instead of taking
 * a variable number of arguments directly, it takes an argument list
 * pointer @p ap.
 */
int vsnprintf(char *restrict buf, size_t maxsize, const char *restrict fmt, va_list ap);

/**
 * @brief Formats a given string and writes the output string to @p str, stopping at @p
 * maxsize
 * @param[out] str The destination buffer to write to.
 * @param[in] maxsize Maximum amount of characters to write.
 * @param[in] fmt The format string to use.
 * @returns The amount of characters written.
 */
int snprintf(char *restrict str, Size maxsize, const char *fmt, ...);

/**
 * @brief Convert a character from uppercase to lowercase
 * @param c The character to convert
 * @return The lowercase version of @p c, or @p c if it couldn't be matched
 */
static inline int tolower(int c) {
        if (c >= 'A' && c <= 'Z') return (c + 0x20);
        return c;
}

/**
 * @brief Convert a character from lowercase to uppercase
 * @param c The character to convert
 * @return The uppercase version of @p c, or @p c if it couldn't be matched
 */
static inline int toupper(int c) {
        if (c >= 'a' && c <= 'z') return (c - 0x20);
        return c;
}

/**
 * @brief Returns whether @p str begins with the characters of @p prefix
 * @param[in] str The string to test
 * @param[in] prefix The prefix to test for
 * @param[in] maxcmp The amount of characters to compare. 0 for null-terminated strings.
 * @returns true if @p str begins with the same characters as @p prefix, false otherwise.
 * @sa strncmp
 */
Bool StringHasPrefix(const char *str, const char *prefix, Size maxcmp);
