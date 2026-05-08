/**********************************************************************
 * FILE: port.h
 * PURPOSE: i8042 port I/O helpers
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>
#define PS2_PORT_DATA    (0x60)
#define PS2_PORT_STATUS  (0x64)
#define PS2_PORT_COMMAND (0x64) /* Same exact port as status port, but for inputs instead. */

/* In busy wait loops, this is how many ticks (increments, in this case) the driver will wait before
 * assuming the hardware won't respond and bail out.*/
#define TIMEOUT_TICKS    (1000000)

#include <io.h>
#include <kernelapi.h>

/**
 * @brief Waits until the output buffer of the i8042 controller is full.
 * @details This function is used whenever data is to be read from the controller. Reading too early
 * may read garbage into memory, so this continuously checks if the output buffer is reported to be
 * full from the controller.
 * @returns STATUS_OK when the output buffer is full, STATUS_TIMEOUT if it was not filled within
 * timeout limits.
 */
static inline Status WaitForOutputBuffer(void) {
        for (int i = 0; i < TIMEOUT_TICKS; i++)
                if ((inb(PS2_PORT_STATUS) & 0x01)) return STATUS_OK;

        return STATUS_TIMEOUT;
}

/**
 * @brief Waits until the input buffer is empty. This function should be used before writing any
 * values to the controller.
 * @returns STATUS_OK when the input buffer is empty, STATUS_TIMEOUT if it was not emptied on time.
 * @sa WaitForOutputBuffer
 */
static inline Status WaitForInputBuffer(void) {
        for (int i = 0; i < TIMEOUT_TICKS; i++)
                if (!(inb(PS2_PORT_COMMAND) & 0x02)) return STATUS_OK;

        return STATUS_TIMEOUT;
}

/** @brief Flushes the contents of the controller port down the drain */
static inline void FlushControllerData(void) {
        while (inb(PS2_PORT_STATUS) & 0x01) inb(PS2_PORT_DATA);
}

/**
 * @brief Writes a command value to the 8042 status port after the input buffer has been
 * cleared. A shortcut for WaitForInputBuffer and outb() synchronization.
 */
static inline void i8042Write(Byte value) {
        if (WaitForInputBuffer() != STATUS_OK) {
#ifdef DRAGONWARE_DEBUG_MODE
                _DWklog(LOG_ERROR,
                        "i8042 input buffer timeout during i8042Write(), not submitting write.");
#endif /* DRAGONWARE_DEBUG_MODE */
                return;
        }
        outb(PS2_PORT_COMMAND, value);
}

/**
 * @brief Writes a value to the 8042 data port after the input buffer has been cleared.
 * Same as @ref i8042Write but uses a different port.
 */
static inline void i8042WriteData(Byte value) {
        if (WaitForInputBuffer() != STATUS_OK) {
#ifdef DRAGONWARE_DEBUG_MODE
                _DWklog(LOG_ERROR,
                        "i8042 input buffer timeout during i8042WriteData(), not submitting "
                        "write.");
#endif /* DRAGONWARE_DEBUG_MODE */
                return;
        }
        outb(PS2_PORT_DATA, value);
}

/**
 * @brief Reads a value from the 8042 controller data port and returns it, only after
 * the controller verifies that the output buffer is full.
 */
static inline Byte i8042Read(void) {
        if (WaitForOutputBuffer() != STATUS_OK) {
#ifdef DRAGONWARE_DEBUG_MODE
                _DWklog(LOG_ERROR,
                        "i8042 output buffer timeout during i8042Write(), returning 0x00.");
#endif /* DRAGONWARE_DEBUG_MODE */
                return 0x00;
        }
        return inb(PS2_PORT_DATA);
}
