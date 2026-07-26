/**********************************************************************
 * FILE: ctype.h
 * PURPOSE: Character tests and conversions helpers according to libc standard
 * PROJECT: DragonWare C Library
 * DATE: 07-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/** XXX Do the functions here need to be linkable at runtime or is static inline also valid? The BSD
 * manpage says that they can also be defined as macros, but I wanted some type safety and cleaner
 * code so I used static inline instead. */

/**
 * @brief Returns whether @p c is an uppercase character or not.
 * @param c The character to check.
 * @returns A nonzero value if @p c is an uppercase character, or zero if it is not.
 */
static inline int isupper(int c) { return (c >= 'A' && c <= 'Z'); }

/**
 * @brief Returns whether @p c is a lowercase character or not.
 * @param c The character to check.
 * @returns A nonzero value if @p c is a lowercase character, or zero if it is not.
 */
static inline int islower(int c) { return (c >= 'a' && c <= 'z'); }

/**
 * @brief Convert a character from uppercase to lowercase
 * @param c The character to convert
 * @return The lowercase version of @p c, or @p c if it couldn't be matched
 */
static inline int tolower(int c) {
        if (isupper(c)) return (c + 0x20);
        return c;
}

/**
 * @brief Convert a character from lowercase to uppercase
 * @param c The character to convert
 * @return The uppercase version of @p c, or @p c if it couldn't be matched
 */
static inline int toupper(int c) {
        if (islower(c)) return (c - 0x20);
        return c;
}

/**
 * @brief Returns whether @p c is a space character or not
 * @param c The character to check
 * @returns A nonzero value if @p c is a space character, or 0 if it is not.
 */
static inline int isspace(int c) {
        return (c == '\t') || (c == ' ') || (c == '\f') || (c == '\n') || (c == '\r') ||
               (c == '\v');
}

/**
 * @brief Returns whether @p c is a decimal digit character
 * @param c The character to check
 * @returns A nonzero value if @p c is a digit, 0 otherwise.
 */
static inline int isdigit(int c) { return (c >= '0' && c <= '9'); }

/**
 * @brief Returns whether @p c is a hexadecimal (base-16) digit character.
 *
 * @param c The character to check
 * @returns A nonzero value if @ref isdigit passes for @p c or @p c is a hexadecimal digit character
 * (A-F).
 */
static inline int isxdigit(int c) {
        int d = tolower(c);
        return isdigit(c) || (d >= 'a' && d <= 'f');
}

/**
 * @brief Returns whether @p c is any printable character except space.
 * @note Spaces are not considered as graphs.
 * @param c The character to check.
 * @returns A nonzero value if @p c is a graph, zero otherwise.
 */
static inline int isgraph(int c) { return (c > 0x20) && (c < 127); }

/**
 * @brief Returns whether @p c is a printable character or not.
 * @note "Printable" in this case refers solely to the ASCII table's structure.
 * @param c The character to check.
 * @returns A nonzero value if @p c is a printable character, zero otherwise.
 */
static inline int isprint(int c) { return isgraph(c) || c == ' '; }

/**
 * @brief Returns whether @p c is an alphabet character or not.
 * @note This is only valid for the "C" locale.
 * @param c The character to check.
 * @returns A nonzero value if @p c is a character of the alphabet, zero otherwise.
 */
static inline int isalpha(int c) { return islower(c) || isupper(c); }

/**
 * @brief Returns whether @p c is a standard blank character.
 * @details Standard blank characters are the regular whitespace (" ") and the tab ("\t").
 * @param c The character to check
 * @returns A nonzero value if @p c is a blank character, zero otherwise.
 */
static inline int isblank(int c) { return (c == ' ') || (c == '\t'); }

/**
 * @brief Returns whether @p c is an alphabetic character or a number.
 * @param c The character to check.
 * @returns A nonzero value if @p c is an alphabetic character or a number, or 0 if it is not.
 */
static inline int isalnum(int c) { return isdigit(c) || isalpha(c); }

/**
 * @brief Returns whether @p c is a punctuation character or not.
 * @param c The character to check.
 * @returns A nonzero value if @p c is a punctuation character, or 0 if it is not.
 */
static inline int ispunct(int c) {
        /*
         * "In the "C" locale, ispunct returns true
         * for every printing character for which neither isspace nor isalnum is true"
         *
         * That's what the standard says, don't ask me
         */
        return isprint(c) && !isspace(c) && !isalnum(c);
}

/**
 * @brief Returns whether @p c is a control character or not.
 * @details The iscntrl() function tests for any control character.
 * The value of the argument must be representable as an unsigned char or
 * the value of EOF.
 *
 * In the ASCII character set, this includes the following characters (with
 * their numeric values shown in octal):
 *
 * 000 NUL       001 SOH       002 STX       003 ETX       004 EOT
 * 005 ENQ       006 ACK       007 BEL       010 BS        011 HT
 * 012 NL        013 VT        014 NP        015 CR        016 SO
 * 017 SI        020 DLE       021 DC1       022 DC2       023 DC3
 * 024 DC4       025 NAK       026 SYN       027 ETB       030 CAN
 * 031 EM        032 SUB       033 ESC       034 FS        035 GS
 * 036 RS        037 US        177 DEL
 *
 * (Taken from the FreeBSD manpage. Again, values above are in octal, not hex or decimal)
 *
 * @param c The character to check.
 * @returns A nonzero value if @p c is a control character, or zero otherwise.
 */
static inline int iscntrl(int c) {
        /* The c >= 0 check is to catch EOF and other bad values. */
        return c >= 0 && (c < 0x20 || c == 127);
}
