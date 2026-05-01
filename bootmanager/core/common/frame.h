/**********************************************************************
 * FILE: frame.h
 * PURPOSE: Physical memory allocating and freeing based on the kernel implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

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
 * @brief Free a previously allocated physical frame.
 * @param frameaddr Physical address of the frame to free.
 */
void FreeFrame(uintptr_t frameaddr);
