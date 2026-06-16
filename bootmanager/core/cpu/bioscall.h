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

/*
 * Needed in real mode, where we have segmented addressing. Briefly, the way to convert a seg:offset address
 * to a linear address is (seg * 16) + offset. These two macros reverse that process by taking a linear address
 * and calculating the segment it is part of and what is the offset.
*/

#define SEGMENT_OF(symbol)           (u16)(((uintptr_t)&(symbol) >> 4U) & 0xFFFFU)
#define OFFSET_IN_SEGMENT_OF(symbol) (u16)((uintptr_t)&(symbol) & 0x000FU)

/**
 * @brief Takes a segmented address (seg:offset scheme) and converts it into a linear 32 bit
 * address.
 * @param[in] addr The address to convert. The format must be as following: High 16 bits are the
 * segment of the address, low 16 bits are the offset within that segment.
 * @returns The linear address calculated from @p addr
 */
static inline uintptr_t SegmentedToLinearAddress(u32 addr) {
        u16 hi = (addr >> 16) & 0xFFFF;
        u16 lo = addr & 0xFFFF;
        return (hi * 16) + lo;
}

#define BIOS_VIDEO_SERVICES   (0x10)
#define BIOS_STORAGE_SERVICES (0x13)
#define BIOS_MEMORY_SERVICES  (0x15)

/**
 * @brief Register state to be loaded when performing a @ref BIOSCall
 * @since v0.0.2
 */
typedef struct [[gnu::packed]] _BIOSRegisters {
        u32 eax, ebx, ecx, edx, esi, edi;
        u32 es, ds;
} BIOSRegisters;

/**
 * @brief Perform a BIOS interrupt in real mode and return the results in @p regs.
 * @param vector Vector number of the BIOS interrupt to perform (eg. 0x13 for disk services, 0x10
 * for video services, and so on)
 * @param[in,out] regs Register state to load. Must not be NullPointer.
 * @returns 0 if the carry flag was not set from the BIOS, 1 if it was set. Many BIOS vectors use
 * the carry flag to indicate success/failure.
 */
[[gnu::nonnull(2)]]
int BIOSCall(int vector, BIOSRegisters *regs);
