/**********************************************************************
 * FILE: pic.h
 * PURPOSE: Programmable Interrupt Controller definitions
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ioport.h>

#define PIC1                  (0x20)
#define PIC2                  (0xA0)
#define PIC1_COMMAND          (PIC1)
#define PIC1_DATA             (PIC1 + 1)
#define PIC2_COMMAND          (PIC2)
#define PIC2_DATA             (PIC2 + 1)

#define ICW1_INIT             (0x10)
#define ICW1_ICW4             (0x01)
#define ICW4_8086_MODE        (0x01)

/* 0x20 = 32, right after our IDT entries */
#define NEW_IRQ_OFFSET_MASTER (0x20)

/* Each controller handles 8 IRQs, so adding 8 gets us to the slave controller being right after our
 * master controller  */
#define NEW_IRQ_OFFSET_SLAVE  (NEW_IRQ_OFFSET_MASTER + 8)

static inline void DisableIRQ(int n) {
        u16  port  = (n < 8) ? PIC1_DATA : PIC2_DATA;
        Byte value = inb(port);
        value |= (1 << n);
        outb(port, value);
}

static inline void EnableIRQ(int n) {
        u16  port  = (n < 8) ? PIC1_DATA : PIC2_DATA;
        Byte value = inb(port);
        value &= ~(1 << n);
        outb(port, value);
}

void RemapPICInterrupts(void);
