/**********************************************************************
 * FILE: gdt.h
 * PURPOSE: Global Descriptor Table definition and helpers.
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define GDT_ENTRIES     6

#define SEL_NULL_DATA   0x0
#define SEL_CODE_KERNEL 0x08
#define SEL_DATA_KERNEL 0x10
#define SEL_CODE_USER   0x1B
#define SEL_DATA_USER   0x23

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _GDTEntry {
        u16  limit_low;
        u16  base_low;
        Byte base_middle;
        Byte access;
        Byte granularity;
        Byte base_high;
} GDTEntry;

typedef struct [[gnu::packed]] _GDTPointer {
        u16 limit;
        u32 base;
} GDTPointer;

void GDTInit(void);

/**
 * @brief Sets the Task State Segment's kernel stack to point to @p kernel_stack
 * @note @p kernel_stack must be 8KBs large and preferably be aligned to 16 bytes.
 * @param stack_top The top of the stack to use as a virtual address (Yes, THE TOP)
 */
void SelectKernelStack(uintptr_t stack_top);
