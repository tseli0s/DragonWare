/**********************************************************************
 * FILE: ps2kbd.c
 * PURPOSE: PS/2 Keyboard Userspace Driver for the DragonWare kernel
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#define PS2_ENABLE_SCANNING   (0xF4)
#define PS2_DISABLE_PORT1     (0xAD)
#define PS2_DISABLE_PORT2     (0xA7)
#define PS2_ENABLE_PORT1      (0xAE)
#define PS2_ENABLE_PORT2      (0xA8)
#define PS2_READ_CONFIG_BYTE  (0x20)
#define PS2_WRITE_CONFIG_BYTE (0x60)
#define PS2_SELF_TEST         (0xAA)
#define PS2_RESET_EVERYTHING  (0xFF)

/*
 * Major TODOs and features for this keyboard server:
 - Implement modifier support
 - Handle non US keyboard layouts
 - (Maybe) support wide characters (in a newer protocol revision) for other scripts
*/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <message.h>
#include <object.h>
#include <string.h>

#include "io.h"
#include "port.h"
#include "protocol.h"
#include "scantochar.h"

static const char ascii_table[128] = {
        [0x00] = 0,    [0x01] = 27,   [0x02] = '1',  [0x03] = '2',  [0x04] = '3',  [0x05] = '4',
        [0x06] = '5',  [0x07] = '6',  [0x08] = '7',  [0x09] = '8',  [0x0A] = '9',  [0x0B] = '0',
        [0x0C] = '-',  [0x0D] = '=',  [0x0E] = '\b', [0x0F] = '\t',

        [0x10] = 'q',  [0x11] = 'w',  [0x12] = 'e',  [0x13] = 'r',  [0x14] = 't',  [0x15] = 'y',
        [0x16] = 'u',  [0x17] = 'i',  [0x18] = 'o',  [0x19] = 'p',  [0x1A] = '[',  [0x1B] = ']',
        [0x1C] = '\n', [0x1D] = 0,

        [0x1E] = 'a',  [0x1F] = 's',  [0x20] = 'd',  [0x21] = 'f',  [0x22] = 'g',  [0x23] = 'h',
        [0x24] = 'j',  [0x25] = 'k',  [0x26] = 'l',  [0x27] = ';',  [0x28] = '\'', [0x29] = '`',

        [0x2A] = 0,    [0x2B] = '\\',

        [0x2C] = 'z',  [0x2D] = 'x',  [0x2E] = 'c',  [0x2F] = 'v',  [0x30] = 'b',  [0x31] = 'n',
        [0x32] = 'm',  [0x33] = ',',  [0x34] = '.',  [0x35] = '/',

        [0x36] = 0,    [0x37] = '*',  [0x38] = 0,    [0x39] = ' ',

        [0x3A] = 0,    [0x3B] = 0,    [0x3C] = 0,    [0x3D] = 0,    [0x3E] = 0,    [0x3F] = 0,
        [0x40] = 0,    [0x41] = 0,    [0x42] = 0,    [0x43] = 0,    [0x44] = 0,

        [0x45] = 0,    [0x46] = 0,

        [0x47] = '7',  [0x48] = '8',  [0x49] = '9',  [0x4A] = '-',  [0x4B] = '4',  [0x4C] = '5',
        [0x4D] = '6',  [0x4E] = '+',  [0x4F] = '1',  [0x50] = '2',  [0x51] = '3',  [0x52] = '0',
        [0x53] = '.',

        [0x54] = 0,    [0x55] = 0,    [0x56] = 0,    [0x57] = 0,    [0x58] = 0,    [0x59] = 0,
        [0x5A] = 0,    [0x5B] = 0,    [0x5C] = 0,    [0x5D] = 0,    [0x5E] = 0,    [0x5F] = 0,

        [0x60] = 0,    [0x61] = 0,    [0x62] = 0,    [0x63] = 0,    [0x64] = 0,    [0x65] = 0,
        [0x66] = 0,    [0x67] = 0,    [0x68] = 0,    [0x69] = 0,    [0x6A] = 0,    [0x6B] = 0,
        [0x6C] = 0,    [0x6D] = 0,    [0x6E] = 0,    [0x6F] = 0,    [0x70] = 0,    [0x71] = 0,
        [0x72] = 0,    [0x73] = 0,    [0x74] = 0,    [0x75] = 0,    [0x76] = 0,    [0x77] = 0,
        [0x78] = 0,    [0x79] = 0,    [0x7A] = 0,    [0x7B] = 0,    [0x7C] = 0,    [0x7D] = 0,
        [0x7E] = 0,    [0x7F] = 0,
};

/* If zero, there's no listener. Kernel processes start from PID 1. */
static u32    listener        = 0;
static Handle listener_handle = -1;

/** @brief Disables the 8042 controller from sending any events while any
 * critical operations are taking place.
 */
static inline void DisableDevicePorts(void) {
        i8042Write(PS2_DISABLE_PORT1);
        i8042Write(PS2_DISABLE_PORT2); /* If the controller is single-channel, this'll be ignored */
}

/** @brief Enables the 8042 controller connected devices, and allows them to dispatch interrupts and
 * receive user input.
 */
static inline void EnableDevicePorts(void) {
        i8042Write(PS2_ENABLE_PORT1);
        i8042Write(PS2_ENABLE_PORT2); /* If the controller is single-channel, this'll be ignored */
}

/**
 * @brief Requests a self test from the 8042 controller and returns whether the self-test
 * passed or not.
 */
static Status Perform8042SelfTest(void) {
        i8042Write(PS2_SELF_TEST);
        Byte result = i8042Read();

        /* Only 0x55 is considered a pass value, any other value is an error */
        if (result != 0x55) {
#ifdef DRAGONWARE_DEBUG_MODE
                _DWklog(LOG_ERROR, "i8042 controller self test failed (result != 0x55)!");
#endif /* DRAGONWARE_DEBUG_MODE */
                return STATUS_BAD;
        }
        return STATUS_OK;
}

/**
 * @brief Attempts to detect the presence of an i8042 controller in the system.
 * @returns STATUS_OK if the controller is present, STATUS_BAD if it could not be detected or/and is
 * faulty.
 * @note This also logs messages in the kernel log buffer.
 */
static Status Probe8042Controller(void) {
        /* DragonWare doesn't support ACPI, so we'll have to test the old fashioned way. */
        DisableDevicePorts();
        FlushControllerData();

        /* controller configuration byte, before we modified it */
        i8042Write(PS2_READ_CONFIG_BYTE);
        Byte old_ccb = i8042Read();
        Byte new_ccb = old_ccb;

        /*
         * Disable IRQ and translation, also make sure clock signal is enabled.
         * See https://wiki.osdev.org/I8042_PS/2_Controller#Initialising_the_PS/2_Controller to
         * understand the bit values here.
         */
        new_ccb &= ~((1 << 0) | (1 << 4) | (1 << 6));
        i8042Write(PS2_WRITE_CONFIG_BYTE);
        i8042WriteData(new_ccb);

        if (Perform8042SelfTest() != STATUS_OK) {
                i8042Write(PS2_WRITE_CONFIG_BYTE);
                i8042WriteData(old_ccb); /* Try to restore the original state we started with,
                                          * in case of faulty hardware.*/
                return STATUS_BAD;
        }

        /* Now reenable IRQs and translation */
        new_ccb |= ((1 << 0) | (1 << 6));
        i8042Write(PS2_WRITE_CONFIG_BYTE);
        i8042WriteData(new_ccb);

        /*
         * Devices must be reenabled for the code below
         * https://wiki.osdev.org/I8042_PS/2_Controller#Initialising_the_PS/2_Controller
         * (Step 9)
         */
        EnableDevicePorts();

        /* Now reset devices, and check if there's any device out there responding to all the
         * configuration we just did */
        i8042WriteData(PS2_RESET_EVERYTHING);
        Byte response = i8042Read();
        if (response != 0xFA) {
                _DWklog(LOG_DEBUG, "Keyboard did not ACK reset request");
                return STATUS_BAD;
        }

        Byte pass = i8042Read();
        if (pass != 0xAA) {
#ifdef DRAGONWARE_DEBUG_MODE
                _DWklog(LOG_ERROR, "Keyboard failed reset self-test.");
#endif
                return STATUS_BAD;
        }

        return STATUS_OK;
}

int main(void) {
        /* We need IO port access from the kernel to read the bytes from the controller. Note that
         * PS2_PORT_COMMAND is the same as PS2_PORT_STATUS in this case so no reason to also specify
         * it */
        u16 ports_needed[] = {PS2_PORT_STATUS, PS2_PORT_DATA};
        if (_DWRequestPorts(ports_needed, 2) != STATUS_OK) return -1;

        if (Probe8042Controller() != STATUS_OK) {
                _DWklog(LOG_ERROR, "Unable to probe i8042 controller. Keyboard will be disabled.");
                return -0xDD;
        }

        i8042WriteData(PS2_ENABLE_SCANNING);

        /* Drain any previously collected events to avoid deadlocks in the PIC */
        FlushControllerData();

        /* Only one keyboard is supported per session for now */
        Handle irqline = CreateObject("KEYBOARD", OBJ_PORT, 0);

        /* IRQ 1 is the PS/2 keyboard in PCs */
        IRQBindingDescriptor bind_descriptor = {
                .irq_no   = 1,
                .reserved = 0,
        };

        if (InvokeObject(irqline, PORT_CREATE, NullPointer) != STATUS_OK) return -1;
        if (InvokeObject(irqline, PORT_BIND_IRQ, &bind_descriptor)) return -1;

        while (true) {
                Message m;
                ReceiveMessage(irqline, &m);
                /* IRQ fired, we need to handle it. */
                if (m.header.sender == KERNEL_SENDER) {
                        if (InvokeObject(irqline, PORT_ACK_IRQ, &bind_descriptor) != STATUS_OK)
                                continue;
                        u8   scancode = 0;
                        char c        = 0;

                        /* Drain ALL scancodes from the port before acknowledging the IRQ. The PS/2
                         * controller is weird, it won't send more interrupts if the port is not
                         * drained for some reason. Or so does a bug I spent hours fixing says. */
                        while (inb(PS2_PORT_STATUS) & 0x01) {
                                scancode = inb(PS2_PORT_DATA);
                                c        = ScancodeToCharacter(ascii_table, scancode);
                        }

                        if (!listener || listener_handle < 0) continue;

                        if (c) {
                                Message keymsg;
                                /* Bytes 2-3 should be zeroed out according to protocol, see
                                 * protocol.h */
                                memset(keymsg.payload.raw + 2, 0, 2);

                                keymsg.header.protocol       = KBD_PROTOCOL_V0;
                                keymsg.header.reply_handle   = -1;
                                keymsg.header.type           = KBD_KEY_DISPATCH;
                                keymsg.header.payload_length = 4;
                                keymsg.payload.raw[0]        = (Byte)c & 0xFF;
                                keymsg.payload.raw[1]        = (Byte)0;

                                SendMessage(listener_handle, &keymsg,
                                            sizeof(keymsg.header) + keymsg.header.payload_length);
                        }

                } else {
                        if (m.header.protocol != KBD_PROTOCOL_V0) continue;
                        switch (m.header.type) {
                                case KBD_LISTENER_REQUEST: {
                                        /* Processes must specify a reply handle to receive
                                         * the status from the server about the request and
                                         * future events. */
                                        if (m.header.reply_handle < 0) break;

                                        /* Check if the protocol is supported or not for
                                         * backwards compatibility in the future. */
                                        if (m.header.protocol != KBD_PROTOCOL_V0) break;

                                        Message reply;
                                        memset(&reply, 0, sizeof(Message));
                                        reply.header.protocol = KBD_PROTOCOL_V0;

                                        if (!listener) {
                                                reply.payload.raw[0]        = STATUS_OK;
                                                reply.header.payload_length = 1;
                                                listener_handle             = m.header.reply_handle;
                                                listener                    = m.header.sender;
                                        } else
                                                reply.payload.raw[0] = (Byte)STATUS_BAD;

                                        SendMessage(m.header.reply_handle, &reply,
                                                    sizeof(reply.header) + 4);
                                        break;
                                }
                                default:
                                        break;
                        }
                }
        }

        return 0;
}
