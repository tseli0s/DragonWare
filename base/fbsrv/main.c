/**********************************************************************
 * FILE: main.c
 * PURPOSE: Framebuffer microkernel server for DragonWare
 * PROJECT: DragonWare Base System
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <message.h>
#include <object.h>
#include <string.h>

#include "fbsrv/console.h"
#include "protocol.h" /* This also defines the vgacons-related protocol data */
#include "ps2kbd/protocol.h"

int main(void) {
        /* The framebuffer server also provides the system console. It may become optional in the
         * far future, but until then, this driver has two responsibilities (And even more under the
         * hood)*/
        Handle              consoleport        = CreateObject("CONSOLE", OBJ_PORT, 0);
        Handle              fbdev              = CreateObject(NullPointer, OBJ_DEVICE, 0);
        Handle              kbdport            = CreateObject(NullPointer, OBJ_PORT, 0);
        Handle              console_controller = -1;
        DeviceMapDescriptor dev;

        if (fbdev < 0 || kbdport < 0 || consoleport < 0) goto cleanup;
        if (InvokeObject(fbdev, DEVICE_GET, "/Devices/Kernel Framebuffer") != STATUS_OK)
                goto cleanup;
        if (InvokeObject(fbdev, DEVICE_CLAIM, &dev) != STATUS_OK) goto cleanup;
        if (InvokeObject(fbdev, DEVICE_MAP, FRAMEBUFFER_ADDR) != STATUS_OK) goto cleanup;

        if (InvokeObject(kbdport, PORT_OPEN, "KEYBOARD") != STATUS_OK) goto cleanup;

        /* Now that we have claimed the framebuffer, ensure our console implementation knows about
         * it */
        RegisterDeviceInfo(&dev);

        Handle kbdreply = CreateObject(NullPointer, OBJ_PORT, 0);
        if (!kbdreply) goto cleanup;

        /* Need to know if the request was accepted or not */
        Message kbdack;
        ReceiveMessage(kbdreply, &kbdack);
        if (kbdack.payload.raw[0] != STATUS_OK) goto cleanup;

        if (InvokeObject(consoleport, PORT_CREATE, NullPointer) != STATUS_OK) goto cleanup;

        Message claim_msg;
        memset(&claim_msg, 0, sizeof(Message));
        claim_msg.header.protocol       = KBD_PROTOCOL_V0;
        claim_msg.header.type           = KBD_LISTENER_REQUEST;
        claim_msg.header.payload_length = 0;
        claim_msg.header.reply_handle   = consoleport;

        SendMessage(kbdport, &claim_msg, sizeof(claim_msg.header));
        while (true) {
                Message m;
                /* If not STATUS_OK, the message was malformed, so we can't trust it */
                if (ReceiveMessage(consoleport, &m) != STATUS_OK) continue;

                switch (m.header.protocol) {
                        case KBD_PROTOCOL_V0: {
                                char c = (char)m.payload.raw[0];

                                /* hackish way to avoid deleting stuff from the screen we shouldnt
                                 * when there's a program running beneath
                                 */
                                if (c != '\b' || (c == '\b' && console_controller < 0)) {
                                        WriteCharacterToConsole(c);
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
                                                strncpy(string, (const char *)m.payload.raw,
                                                        m.header.payload_length);
                                                string[m.header.payload_length] = '\0';
                                                WriteStringToConsole(string);
                                                break;
                                        }
                                        case VGACONS_REQUEST_CHAR_DRAW: {
                                                char c = (char)m.payload.raw[0];
                                                WriteCharacterToConsole(c);
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
cleanup:
        if (consoleport >= 0) DeleteObject(consoleport);
        if (fbdev >= 0) DeleteObject(fbdev);
        if (kbdport >= 0) DeleteObject(kbdport);
        return -1;
}
