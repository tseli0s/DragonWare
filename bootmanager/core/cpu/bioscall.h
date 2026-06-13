/**********************************************************************
 * FILE: bioscall.h
 * PURPOSE: BIOS call helper exports
 * PROJECT: DragonWare Boot Manager
 * DATE: 06-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Register state to be loaded when performing a @ref BIOSCall
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _BIOSRegisters {
        u32 eax, ebx, ecx, edx, esi, edi;
        u32 es, ds;
} BIOSRegisters;

/**
 * @brief Perform a BIOS interrupt in real mode. 
 * @todo Return any results to @p regs from the BIOS (currently nothing is returned), and check if CF is set (Used mostly to check if the call failed or not)
 * @param vector Vector number of the BIOS interrupt to perform (eg. 0x13 for disk services, 0x10 for video services, and so on)
 * @param[in] regs Register state to load. Must not be NullPointer.
 */
void BIOSCall(int vector, BIOSRegisters *regs);
