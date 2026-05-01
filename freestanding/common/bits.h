/**********************************************************************
 * FILE: bits.h
 * PURPOSE: Bitwise operation helpers and utilities
 * PROJECT: DragonWare Freestanding Library
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Set bit @p n of @p x to 1
 * @param[in] x The integer to set the bit of
 * @param[in] n The bit to set
 */
#define SET_BIT(x, n)    ((x) |= (1U << (n)))

/**
 * @brief Clear bit @p n of @p x
 * @param[in] x The integer to clear the bit of
 * @param[in] n The bit to clear
 */
#define CLEAR_BIT(x, n)  ((x) &= ~(1U << (n)))

/**
 * @brief Toggle (switch) bit @p n of @p x from 0 to 1 or from 1 to 0 depending on the current value
 * of bit @p n
 * @param[in] x The integer to toggle the bit of
 * @param[in] n The bit to toggle
 */
#define TOGGLE_BIT(x, n) ((x) ^= (1U << (n)))

/**
 * @brief Checks whether a bit of integer @p x is set or not.
 * @param[in] x The integer to check
 * @param[in] n The bit position to check for
 * @note This is how flags can be checked in a bitfield of flags, where @p x would be the flags, and
 * @p n would be the flag to test for.
 */
#define IS_SET(x, n)     (!!((x) & (1U << (n))))
