/**********************************************************************
 * FILE: pic.c
 * PURPOSE: 8259 Programmable Interrupt Controller initialization and remapping
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "pic.h"

#include <ioport.h>

#include "irq.h"

/* Older machines needed their time when configuring the PIC. */
static inline void IOWait(void) {
        outb(0x80, 0);
        outb(0x80, 0);
}

void DisableIRQ(unsigned int n) {
        if (unlikely(!inrange(n, 0, MAX_IRQ - 1))) return;
        u16  port  = (n < 8) ? PIC1_DATA : PIC2_DATA;
        Byte value = inb(port);
        value |= (Byte)(1u << (n % 8));
        outb(port, value);
}

void EnableIRQ(unsigned int n) {
        if (unlikely(!inrange(n, 0, MAX_IRQ - 1))) return;
        u16  port  = (n < 8) ? PIC1_DATA : PIC2_DATA;
        Byte value = inb(port);
        /* Normalize the n so it is always between 0-7 for the target PIC
         * There was a bug where the bit shifting here caused value to become 0 for IRQs >7, just a
         * reminder to myself to be more careful with bit shifting */
        value &= (Byte) ~(1u << (n % 8));
        outb(port, value);

        /* IRQ2 is used by the slave PIC, so if an IRQ>7 is enabled, we must also unmask the slave
         * PIC to receive interrupts from it */
        if (n > 7) EnableIRQ(2);
}

void RemapPICInterrupts(void) {
        outb(PIC1_DATA, NEW_IRQ_OFFSET_MASTER);
        IOWait();
        outb(PIC2_DATA, NEW_IRQ_OFFSET_SLAVE);
}

Status InitializePIC(void) {
        /* Enable the PICs (1 and 2) in cascade mode */
        outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
        IOWait();
        outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
        IOWait();

        RemapPICInterrupts();

        /*
         * Tell the master PIC where the slave is (after the remapping has been done)
         * TODO: Don't use magic numbers here.
         */
        outb(PIC1_DATA, 0x04);
        outb(PIC2_DATA, 0x02);
        IOWait();

        /* Older PICs ran in an incompatible 8080 mode. We want the 8086 mode which is compatible
         * with the protected mode of our kernel. */
        outb(PIC1_DATA, ICW4_8086_MODE);
        outb(PIC2_DATA, ICW4_8086_MODE);
        IOWait();

        /* Mask all the IRQs. We will configure them on demand. */
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);

        return STATUS_OK;
}
