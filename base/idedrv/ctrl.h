/**********************************************************************
 * FILE: ctrl.h
 * PURPOSE: IDE/ATA bus control functions
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <io.h>

#include "portdef.h"

/**
 * @brief Disables interrupts coming from the IDE bus @p bus
 * @param bus Bus to disable interrupts to (0 for primary, 1 for secondary bus).
 */
static inline void DisableINTRQ(int bus) {
        u16 port = (bus == 0) ? ATA_CTRL_PRIMARY : ATA_CTRL_SECONDARY;
        outb(port, 0x02); /* bit 1 of the control register controls interrupts */
}

/**
 * @brief Enables interrupts coming from the IDE bus @p bus
 * @param bus Bus to enable interrupts to (0 for primary, 1 for secondary bus).
 */
static inline void EnableINTRQ(int bus) {
        u16 port = (bus == 0) ? ATA_CTRL_PRIMARY : ATA_CTRL_SECONDARY;
        outb(port, 0x00);
}
