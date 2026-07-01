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

/**
 * @brief Copies @p size amount of bytes from @p src to @p dest following the semantics of the C
 * standard.
 * @note Overlapping pointers are not considered by this function. If @p src and @p dest overlap, it
 * is undefined behaviour.
 * @param[out] dest Pointer to the address to copy the data to.
 * @param[in] src Pointer to the address to copy from.
 * @param size Amount of bytes to copy.
 * @returns @p dest
 */
void *memcpy(void *dest, const void *src, size_t size);

/**
 * @brief Copies @p size bytes from @p src to @p dest while safely handling overlapping regions.
 * @note The difference between this and @ref memcpy is the way they handle overlapping pointers.
 * Passing two overlapping pointers in @ref memcpy is undefined behaviour, but memmove checks if the
 * two pointers overlap and copies the data in a safe way if they do.
 * @param[out] dest Pointer to the address to copy to. Passing @ref NULL is undefined behaviour.
 * @param[in] src Pointer to the address to copy from. Passing @ref NULL is undefined behaviour.
 * @param size Amount of bytes to copy.
 * @sa memcpy
 * @returns @p dest
 */
void *memmove(void *dest, const void *src, size_t size);

/**
 * @brief Starting at the memory address pointed to by @p dest, writes @p value to @p dest for @p
 * size bytes.
 * @note Only the lower eight bits of @p value are considered in the store operation.
 * @param[out] dest Address to write @p value to.
 * @param value Value to write.
 * @param size Amount of @p value to write starting at @p dest
 * @returns @p dest
 */
void *memset(void *dest, int value, size_t size);

/**
 * @brief Compares @p n bytes and returns the result
 * @param[in] p1 Pointer to the first object to compare.
 * @param[in] p2 Pointer to the second object to compare.
 * @param n Amount of bytes to compare.
 * @returns A negative value if the first different byte in @p p1 is less than @p p2, a positive
 * value if the first different byte in @p p1 is greater than @p p2 or 0 if the @p n bytes of @p p1 and
 * @p p2 are equal.
 */
int memcmp(const void *p1, const void *p2, size_t n);

/**
 * @brief Copies @p src to @p dest up until finding a NULL byte.
 * @param[out] dest Destination to copy the string to.
 * @param[in] src Source string to copy from.
 * @note The NULL terminating byte is copied as well and does not need to be written manually at the end of @p dest by the caller.
 * @returns @p dest
 */
char *strcpy(char *dest, const char *src);

/**
 * @brief Copies @p src to @p dest up until finding a NULL byte OR reaching @p size without finding
 * a NULL byte. In case a NULL byte was found in @p src within @p size bytes, then the
 * rest of @p dest is filled with zeroes.
 * @param[out] dest Destination to copy the string to.
 * @param[in] src Source string to copy from.
 * @param size Amount of bytes to copy.
 * @returns @p dest
 */
char *strncpy(char *dest, const char *src, size_t size);

char *strcat(char *dest, const char *src);

char *strncat(char *dest, const char *src, size_t n);

/**
 * @brief Compares two NULL terminated strings and returns the result.
 * @param[in] s1 First string to compare.
 * @param[in] s2 Second string to compare.
 * @returns A positive value if @p s1 is greater than @p s2, a negative value of @p s1 is lesser
 * than @p s2, or 0 if the two strings are equal.
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Compares two NULL terminated strings and returns the result, for up to @p n bytes or until
 * reaching the NULL terminator of one of them.
 * @param[in] s1 First string to compare.
 * @param[in] s2 Second string to compare.
 * @param n Amount of characters to compare.
 * @returns A positive value if the character of @p s1 is greater than the character of @p s2 at the
 * offset where the two strings differ, a negative value if the character of @p s1 is lesser than
 * the character of @p s2 at the offset where the two strings differ, or 0 if the two strings are
 * equal up until @p n bytes.
 * @sa strncmp
 */
int strncmp(const char *s1, const char *s2, size_t n);

/**
 * @brief Searches for a given character within @p s and returns a pointer within @p s pointing to
 * the first occurence of that byte.
 * @param[in] s String to search into.
 * @param c Character to search for.
 * @returns Pointer to the character @p c within @p s or @ref NULL if the character @p c was not found
 */
char *strchr(const char *s, int c);

/**
 * @brief Returns the length of the NULL terminated string given.
 * @param[in] s String to get the length of.
 * @returns Length of @p s without including the NULL terminator.
 */
size_t strlen(const char *s);

DLC_END_DECLS
