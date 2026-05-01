/**********************************************************************
 * FILE: ps2kbd.c
 * PURPOSE: PS/2 Keyboard Userspace Driver for the DragonWare kernel
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#define PS2_PORT_DATA       (0x60)
#define PS2_PORT_STATUS     (0x64)
#define PS2_ENABLE_SCANNING (0xF4)

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

int main(void) {
        /* We need IOPL support from the kernel to read the bytes from the controller. */
        if (_DWRaiseIOPL() != STATUS_OK) return -1;
        outb(PS2_PORT_DATA, PS2_ENABLE_SCANNING);

        /* Drain any previously collected events to avoid deadlocks in the PIC */
        while (inb(PS2_PORT_STATUS) & 0x01) inb(PS2_PORT_DATA);

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
