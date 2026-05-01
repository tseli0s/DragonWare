/**********************************************************************
 * FILE: irq.c
 * PURPOSE: IRQ installation and handling
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "irq.h"

#include <ktypes.h>

#include "idt86.h"
#include "pic.h"

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

static IRQHandler IRQHandlerCallbacks[MAX_IRQ] = {0};

static void IRQInstall(void) {
        const u16 kernelcode = 0x08;
        IDTAddGate(32, (u32)irq0, kernelcode, 0x8E);
        IDTAddGate(33, (u32)irq1, kernelcode, 0x8E);
        IDTAddGate(34, (u32)irq2, kernelcode, 0x8E);
        IDTAddGate(35, (u32)irq3, kernelcode, 0x8E);
        IDTAddGate(36, (u32)irq4, kernelcode, 0x8E);
        IDTAddGate(37, (u32)irq5, kernelcode, 0x8E);
        IDTAddGate(38, (u32)irq6, kernelcode, 0x8E);
        IDTAddGate(39, (u32)irq7, kernelcode, 0x8E);
        IDTAddGate(40, (u32)irq8, kernelcode, 0x8E);
        IDTAddGate(41, (u32)irq9, kernelcode, 0x8E);
        IDTAddGate(42, (u32)irq10, kernelcode, 0x8E);
        IDTAddGate(43, (u32)irq11, kernelcode, 0x8E);
        IDTAddGate(44, (u32)irq12, kernelcode, 0x8E);
        IDTAddGate(45, (u32)irq13, kernelcode, 0x8E);
        IDTAddGate(46, (u32)irq14, kernelcode, 0x8E);
        IDTAddGate(47, (u32)irq15, kernelcode, 0x8E);
}

void RegisterIRQHandler(int which, IRQHandler handler) {
        /* assume that the handler should be removed */
        if (!handler) {
                DisableIRQ(which);
                return;
        }
        if (which <= MAX_IRQ) {
                IRQHandlerCallbacks[which] = handler;
                EnableIRQ(which);
        }
}

void IRQHandlerCallback(IRQRegisters *r) {
        u32 irq = r->int_no - 0x20; /* IRQs come right after all the 32 (0x20) IDT entries */

        if (irq < MAX_IRQ) {
                IRQHandler handler = IRQHandlerCallbacks[irq];
                if (handler) handler(r);
        }
}

void IRQInit(void) {
        RemapPICInterrupts();
        IRQInstall();
}
