/**********************************************************************
 * FILE: abs.h
 * PURPOSE: Absolute function definitions
 * PROJECT: DragonWare Freestanding Library
 * DATE: 11-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/**
 * @brief Returns the absolute value of @p n (|n| in mathematical notation).
 * @since v0.0.2
 * @note If @p n = @ref INT_MIN then this function returns INT_MIN as well, because the absolute value of INT_MIN cannot fit into a C integer in most cases.
 * @warning This assumes @p n uses two's complement for the sign.
 */
int abs(int n);

/**
 * @brief Returns the absolute value of @p n (|n| in mathematical notation).
 * @since v0.0.2
 * @note If @p n = @ref LONG_MIN then this function returns LONG_MIN as well, because the absolute value of INT_MIN cannot fit into a C integer in most cases.
 * @warning This assumes @p n uses two's complement for the sign.
 */
long labs(long n);

/**
 * @brief Returns the absolute value of @p n (|n| in mathematical notation).
 * @since v0.0.2
 * @note If @p n = @ref LLONG_MIN then this function returns LLONG_MIN as well, because the absolute value of INT_MIN cannot fit into a C integer in most cases.
 * @warning This assumes @p n uses two's complement for the sign.
 */
long long llabs(long long n);