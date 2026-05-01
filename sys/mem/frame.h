/**********************************************************************
 * FILE: frame.h
 * PURPOSE: Physical frame memory management and constants
 * PROJECT: DragonWare Kernel
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/
#pragma once

#include <ktypes.h>
#include <macros.h>

#define FRAME_SIZE (0x1000)

/**
 * @brief Initialize the physical frame manager.
 * @details This function sets up the internal structures required to track and
 * allocate physical memory frames.
 */
void InitFrameManager(void);

/**
 * @brief Allocate a single physical memory frame.
 * @details The returned address is always in high memory (> 1 MiB). For allocations
 * in low memory, use @ref AllocateLowMemory.
 * @returns Physical address of the allocated frame, or 0 on failure.
 */
uintptr_t AllocateFrame(void);

/**
 * @brief Allocate a single physical memory frame in low memory.
 * @details Use this function when a physical address below 1 MiB is required,
 * such as for certain legacy hardware or bootloader requirements.
 * @returns Physical address of the allocated low memory frame, or 0 on failure.
 */
uintptr_t AllocateLowMemory(void);

/**
 * @brief Free a previously allocated physical frame.
 * @param frameaddr Physical address of the frame to free.
 */
void FreeFrame(uintptr_t frameaddr);
