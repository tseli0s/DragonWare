/**********************************************************************
 * FILE: idt.h
 * PURPOSE: Interrupt Descriptor Table (IA32) public interface
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define IDT_ENTRIES (256)

/* I would like to use 0x80 for some POSIX syscalls instead, just in case */
#define SYSCALL_NO  (0x60)

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _IDTEntry {
        u16  base_low;
        u16  selector;
        Byte zero; /* Why the fuck do we need a field that should always be
                         zero? */
        Byte flags;
        u16  base_high;
} IDTEntry;

typedef struct [[gnu::packed, gnu::aligned(16)]] _IDTPointer {
        u16 limit;
        u32 base;
} IDTPointer;

void IDTAddGate(int n, u32 base, u16 selector, u16 flags);
void IDTInit(void);

/**
 * @brief Disables interrupts and increments the critical counter
 * @note The critical counter is an internal value. It tracks how many times interrupts were
 * enabled and disabled in critical sections. This is effectively a "do your job without worrying
 * that another thread will run at the same time"
 * @credits RetrOS-32 (github.com/joexbayer/RetrOS-32)
 */
void PushCritical(void);

/**
 * @brief Decrements the internal critical counter. If that counter reaches 0, interrupts are
 * enabled automatically.
 * @note Use this in combination with @ref PushCritical
 * @credits RetrOS-32 (github.com/joexbayer/RetrOS-32)
 */
void PopCritical(void);
