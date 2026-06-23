/**********************************************************************
 * FILE: console.h
 * PURPOSE: Framebuffer-based console implementation
 * PROJECT: DragonWare Base System
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define FRAMEBUFFER_ADDR ((void *)(0x60000000))

#include <kernelapi.h>

/**
 * @brief Registers device information for the framebuffer in the internal server structures.
 * @param[in] dev Device descriptor to extract information from. Cannot be NullPointer.
 */
[[gnu::nonnull]]
void RegisterDeviceInfo(DeviceMapDescriptor *dev);

/**
 * @brief Print a character to the framebuffer console in the current position.
 * @param c The character to print. Must be ASCII.
 */
void WriteCharacterToConsole(char c);

/**
 * @brief Prints a string to the console in the current position.
 * @note A newline is NOT printed automatically.
 * @param[in] str String to write. Must be NULL-terminated. Cannot be NULL.
 */
[[gnu::nonnull]]
void WriteStringToConsole(const char *str);
