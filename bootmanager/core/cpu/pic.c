/**********************************************************************
 * FILE: pic.c
 * PURPOSE: Programmable Interrupt Controller initialization code
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "pic.h"

#include <ioport.h>

/* Older machines needed their time when configuring the PIC. */
static inline void IOWait(void) { outb(0x80, 0); }

void RemapPICInterrupts(void) {
        /* Enable the PICs (1 and 2) in cascade mode */
        outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
        outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
        IOWait();

        /* Tell the controllers we want to receive IRQs at indexes higher than the default IDT
         * entries */
        outb(PIC1_DATA, NEW_IRQ_OFFSET_MASTER);
        outb(PIC2_DATA, NEW_IRQ_OFFSET_SLAVE);
        IOWait();

        /* Tell the master PIC where the slave is */
        outb(PIC1_DATA, 4);
        outb(PIC2_DATA, 2);
        IOWait();

        /* Older PICs ran in an incompatible 8080 mode. We want the 8086 mode which is compatible
         * with the protected mode of our kernel. */
        outb(PIC1_DATA, ICW4_8086_MODE);
        outb(PIC2_DATA, ICW4_8086_MODE);
        IOWait();

        /* Mask all the IRQs. We will configure them on demand. */
        outb(PIC1_DATA, 0xFF);
        outb(PIC2_DATA, 0xFF);
        IOWait();
}
