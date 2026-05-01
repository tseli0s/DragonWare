/**********************************************************************
 * FILE: console.c
 * PURPOSE: VGA text mode console driver in userspace
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

#include <ipc86.h>
#include <kernelapi.h>
#include <kerneltypes.h>
#include <message.h>
#include <object.h>
#include <string.h>

#include "driver.h"
#include "protocol.h"
#include "ps2kbd/protocol.h"

#define VGA_CONSOLE_WIDTH  (80)
#define VGA_CONSOLE_HEIGHT (25)

int main(void) {
        /* Need IOPL permissions to control the VGA cursor */
        if (_DWRaiseIOPL() != STATUS_OK) return -1;

        /* Device object, to claim the VGA text mode driver from the kernel */
        Handle consoledev = CreateObject(NullPointer, OBJ_DEVICE, 0);
        Handle kbdport    = CreateObject(NullPointer, OBJ_PORT, 0);

        DeviceMapDescriptor dev;

        if (consoledev < 0 || kbdport < 0) goto cleanup;
        if (InvokeObject(consoledev, DEVICE_GET, "/Devices/VGA Console") != STATUS_OK) goto cleanup;
        if (InvokeObject(consoledev, DEVICE_CLAIM, &dev) != STATUS_OK) goto cleanup;
        if (InvokeObject(consoledev, DEVICE_MAP, VGA_CONSOLE_VIRTUAL_ADDR) != STATUS_OK)
                goto cleanup;

        if (InvokeObject(kbdport, PORT_OPEN, "KEYBOARD") != STATUS_OK) goto cleanup;

        Handle kbdreply = CreateObject(NullPointer, OBJ_PORT, 0);
        if (!kbdreply) goto cleanup;

        /* Need to know if the request was accepted or not */
        Message kbdack;
        ReceiveMessage(kbdreply, &kbdack);
        if (kbdack.payload.raw[0] != STATUS_OK) goto cleanup;

        /* Clear the screen now that the server owns it. */
        VGAClearAllText(VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY, VGATEXT_COLOR_BLACK));

        Handle replyport = CreateObject("CONSOLE", OBJ_PORT, 0);
        if (InvokeObject(replyport, PORT_CREATE, NullPointer) != STATUS_OK) goto cleanup;
        LogStatus("CONSOLE port online.");

        Message claim_msg;
        memset(&claim_msg, 0, sizeof(Message));
        claim_msg.header.protocol       = KBD_PROTOCOL_V0;
        claim_msg.header.type           = KBD_LISTENER_REQUEST;
        claim_msg.header.payload_length = 0;
        claim_msg.header.reply_handle   = replyport;

        SendMessage(kbdport, &claim_msg, sizeof(claim_msg.header));

        Handle console_controller = -1;
        LogStatus("Waiting for child connections, please wait...");

        VGAPrintString("---------------------------------------------------------------------\n\n",
                       VGAGetColorAttribute(VGATEXT_COLOR_WHITE, VGATEXT_COLOR_BLACK));
        while (true) {
                Message m;
                /* If not STATUS_OK, the message was malformed, so we can't trust it */
                if (ReceiveMessage(replyport, &m) != STATUS_OK) continue;

                switch (m.header.protocol) {
                        case KBD_PROTOCOL_V0: {
                                char c = (char)m.payload.raw[0];

                                /* hackish way to avoid deleting stuff from the screen we shouldnt
                                 * when there's a program running beneath
                                 */
                                if (c != '\b' || (c == '\b' && console_controller < 0)) {
                                        VGAPrintCharacter(
                                                c, VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY,
                                                                        VGATEXT_COLOR_BLACK));
                                }

                                if (console_controller >= 0)
                                        SendMessage(console_controller, &m,
                                                    sizeof(m.header) + m.header.payload_length);
                                break;
                        }
                        case VGACONS_PROTOCOL_V0:
                                switch (m.header.type) {
                                        case VGACONS_CLAIM_CONSOLE: {
                                                if (m.header.reply_handle < 0) break;

                                                Status reply = STATUS_BAD;
                                                if (console_controller >= 0)
                                                        break;
                                                else {
                                                        reply              = STATUS_OK;
                                                        console_controller = m.header.reply_handle;
                                                }
                                                Message replymsg;
                                                replymsg.header.payload_length = 1;
                                                replymsg.header.type           = m.header.type;
                                                replymsg.header.protocol     = VGACONS_PROTOCOL_V0;
                                                replymsg.header.reply_handle = -1;
                                                replymsg.payload.raw[0]      = (Byte)reply;
                                                SendMessage(m.header.reply_handle, &replymsg,
                                                            sizeof(replymsg.header) + sizeof(Byte));

                                                break;
                                        }
                                        case VGACONS_REQUEST_STRING_DRAW: {
                                                char string[MESSAGE_BUFFER_SIZE];
                                                strncpy(string, (const char*)m.payload.raw,
                                                        m.header.payload_length);
                                                string[m.header.payload_length] = '\0';
                                                VGAPrintString(string,
                                                               VGAGetColorAttribute(
                                                                       VGATEXT_COLOR_LIGHT_GREY,
                                                                       VGATEXT_COLOR_BLACK));
                                                break;
                                        }
                                        default:
                                                break;
                                }
                                break;
                        default:
                                break;
                }
        }

        return 0;
/* The kernel internally ignores negative handle IDs to make this pattern work */
cleanup:
        DeleteObject(consoledev);
        DeleteObject(kbdport);
        return -1;
}
