/**********************************************************************
 * FILE: idt86.h
 * PURPOSE: Interrupt Descriptor Table (IA32) public interface
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>
#include <macros.h>

typedef struct [[gnu::packed]] _IDTRegisters {
        u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
        u32 int_no;
        u32 err_code;
        u32 eip, cs, eflags;
} IDTRegisters;

typedef struct [[gnu::packed]] _IDTEntry {
        u16  base_low;
        u16  selector;
        Byte zero; /* Why the fuck do we need a field that should always be
                         zero? */
        Byte flags;
        u16  base_high;
} IDTEntry;

typedef struct [[gnu::packed, gnu::aligned(2)]] _IDTPointer {
        u16 limit;
        u32 base;
} IDTPointer;

void IDTAddGate(int n, u32 base, u16 selector, u16 flags);
void IDTInit(void);
