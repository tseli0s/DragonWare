/**********************************************************************
 * FILE: com.c
 * PURPOSE: Serial port output driver for DragonWare (Debug mode only)
 * PROJECT: DragonWare Freestanding Library
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ioport.h>
#include <kmalloc.h>
#include <ktypes.h>
#include <macros.h>

#include "drivers/driversdk.h"
#include "iomgr/class.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "log.h"
#include "video/pixels.h"

#define COM1_PORT (0x3F8)

#ifndef __K_DISABLE_SERIAL_OUTPUT

static inline void WaitForPort1(void) { while (!(inb(COM1_PORT + 5) & 0x20)); }

#ifndef __K_DISABLE_SERIAL_OUTPUT

static inline void WaitForPort(void) { while (!(inb(COM1_PORT + 5) & 0x20)); }

static inline void WriteToSerialPort(char c) {
        WaitForPort();
        outb(COM1_PORT, (Byte)c);
}

#endif /* __K_DISABLE_SERIAL_OUTPUT */

/* We can't control the colors of a serial port, so just have a dummy function that's like "yeah bro
 * totally I'm gonna show the text in red from the other side of the world. Rainbow gradient,
 * even..."*/
static void _SetTextAttributes(void *private, PixelColor bg, PixelColor fg) {
        UnusedParameter(private);
        UnusedParameter(fg);
        UnusedParameter(bg);
}

[[gnu::hot]]
static void WriteSerialChar(void *private, char c) {
        UnusedParameter(private);
#ifndef __K_DISABLE_SERIAL_OUTPUT
        if (c == '\n') {
                WriteToSerialPort('\r');
                WriteToSerialPort('\n');
        } else
                WriteToSerialPort(c);
#else
        UnusedParameter(c);
#endif
}

static void ResetSerialConsole(void *private_state) { UnusedParameter(private_state); }

typedef struct _DriverState {
        Bool com1_enabled;
} DriverState;

Status Serial86Init(void) {
        LogMessage(LOG_INFO,
                   "Starting serial driver for DragonWare (Support for port COM1 built in)");
#if defined(__K_DISABLE_SERIAL_OUTPUT) || !defined(DRAGONWARE_DEBUG_MODE)
        LogMessage(LOG_INFO,
                   "Kernel not in debug mode or support for serial output was disabled at build "
                   "time. Nothing to do.");
        return STATUS_OK;
#endif
        DeviceManagerNode *node =
                MakeDeviceNode("Serial Port Driver", P_MUTABLE | P_HAVE_CHILDREN | P_USER,
                               DEVCLASS_UART | DEVCLASS_CONSOLE);
        if (!node) return STATUS_OUT_OF_MEMORY;

        node->devtable.ddo = kzalloc(sizeof(DeviceOperations));
        if (!node->devtable.ddo) {
                kfree(node);
                return STATUS_OUT_OF_MEMORY;
        }

        DriverState *state = kzalloc(sizeof(DriverState));
        if (!state) {
                kfree(node->devtable.ddo);
                kfree(node);
                return STATUS_OUT_OF_MEMORY;
        }

        ConsoleDeviceOps console_ops = {.WriteSingleChar   = WriteSerialChar,
                                        .SetTextAttributes = _SetTextAttributes,
                                        .ResetConsole      = ResetSerialConsole,
                                        .DeleteSingleChar  = NullPointer};
        UARTDeviceOps    uart_ops    = {
                      .WriteSingleChar = WriteSerialChar, .ReceiveByteFromSerial = NullPointer /* TODO */
        };

        state->com1_enabled         = true;
        node->devtable.ddo->uart    = uart_ops;
        node->devtable.ddo->console = console_ops;

        node->private_state = state;
        AddDevice(NullPointer, node);
        return STATUS_OK;
}

const DriverDescriptor ser86_descriptor = {.name         = "Serial Port Driver (COM1/COM2)",
                                           .author       = "DragonWare",
                                           .license      = "GPLv3.0",
                                           .init_earlier = true,
                                           .__init       = &Serial86Init,
                                           .__delete     = NullPointer};
ADD_DRIVER_DESCRIPTOR(ser86_descriptor);
