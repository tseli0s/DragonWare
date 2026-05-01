/**********************************************************************
 * FILE: pic.h
 * PURPOSE: 8259 Programmable Interrupt Controller initialization and remapping function(s)
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ioport.h>
#include <ktypes.h>

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

/**
 * @brief Disables an IRQ from being dispatched to the kernel by configuring the relevant PIC ports.
 * @param[in] n The IRQ to disable. Must be in range 0-15.
 */
void DisableIRQ(unsigned int n);

/**
 * @brief Enables an IRQ and allows it to interrupt kernel execution to be served.
 * @param[in] n The IRQ to enable. Must be within range 0-15.
 */
void EnableIRQ(unsigned int n);

/**
 * @brief Remaps the PIC IRQs to IDT vectors higher than the default range.
 * @sa InitializePIC
 */
void RemapPICInterrupts(void);

/**
 * @brief Initializes the Programmable Interrupt Controller (i8259) and prepares it for future IRQ
 * dispatching.
 * @note This does not enable interrupts.
 * @return STATUS_OK if the initialization was successful. STATUS_BAD if the initialization couldn't
 * be done.
 */
Status InitializePIC(void);
