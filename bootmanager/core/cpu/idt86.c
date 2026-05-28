/**********************************************************************
 * FILE: idt86.c
 * PURPOSE: Interrupt Descriptor Table (IA32) implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "idt86.h"

#include <ktypes.h>
#include <power.h>

#include "error.h"
#include "irq.h"
#include "isr_auto.h"

#define RING0_FLAG     (0x8E)
#define GDT_CS_SEGMENT (0x08)
#define IDT_ENTRIES    (256)

static IDTEntry   idt[IDT_ENTRIES] = {0};
static IDTPointer idtptr           = {0};

static inline void __lidt(u32 idtp) {
        __asm__ volatile(
                "mov %0, %%eax\n"
                "lidt (%%eax)\n" ::"r"(idtp)
                : "eax");
}

void IDTAddGate(int n, u32 base, u16 selector, u16 flags) {
        idt[n].base_low  = base & 0xFFFF;
        idt[n].base_high = (u16)((base >> 16) & 0xFFFF);
        idt[n].selector  = selector;
        idt[n].zero      = 0;
        idt[n].flags     = (Byte)flags;
}

static void FallbackHandler(void) { StallMachine(); }

static void isr_install(void) {
        IDTAddGate(0, (u32)isr0, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(1, (u32)isr1, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(2, (u32)isr2, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(3, (u32)isr3, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(4, (u32)isr4, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(5, (u32)isr5, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(6, (u32)isr6, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(7, (u32)isr7, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(8, (u32)isr8, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(9, (u32)isr9, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(10, (u32)isr10, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(11, (u32)isr11, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(12, (u32)isr12, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(13, (u32)isr13, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(14, (u32)isr14, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(15, (u32)isr15, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(16, (u32)isr16, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(17, (u32)isr17, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(18, (u32)isr18, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(19, (u32)isr19, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(20, (u32)isr20, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(21, (u32)isr21, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(22, (u32)isr22, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(23, (u32)isr23, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(24, (u32)isr24, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(25, (u32)isr25, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(26, (u32)isr26, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(27, (u32)isr27, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(28, (u32)isr28, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(29, (u32)isr29, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(30, (u32)isr30, GDT_CS_SEGMENT, RING0_FLAG);
        IDTAddGate(31, (u32)isr31, GDT_CS_SEGMENT, RING0_FLAG);

        for (int i = 32; i < IDT_ENTRIES; i++) {
                IDTAddGate(i, (u32)FallbackHandler, GDT_CS_SEGMENT, RING0_FLAG);
        }
}

void isr_handler(IDTRegisters *r) {
        FatalError("CPU Exception %d: eip 0x%d, error_code 0x%x, eflags: 0x%x, stack: 0x%x",
                   r->int_no, r->eip, r->err_code, r->eflags, r->esp);
        StallMachine();
}

void IDTInit(void) {
        isr_install();

        idtptr.limit = (sizeof(IDTEntry) * IDT_ENTRIES) - 1;
        idtptr.base  = (u32)&idt;

        __lidt((u32)&idtptr);
        IRQInit();
        __asm__ volatile("sti");
}
