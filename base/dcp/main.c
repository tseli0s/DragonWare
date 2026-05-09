/**********************************************************************
 * FILE: main.c
 * PURPOSE: DragonWare Command Prompt implementation
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ipc86.h>
#include <kernelapi.h>
#include <kerneltypes.h>
#include <message.h>
#include <object.h>
#include <stdio.h>
#include <string.h>

#include "dcp/command.h"
#include "ps2kbd/protocol.h"
#include "vgacons/protocol.h"

#define MAXCOMMAND (1024)

/* Close enough for me */
static inline Bool CharacterIsPrintable(char c) { return (c >= 0x20); }

int main(void) {
        Handle consport            = CreateObject(NullPointer, OBJ_PORT, 0);
        Handle commport            = CreateObject(NullPointer, OBJ_PORT, 0);
        char   command[MAXCOMMAND] = {0};
        Size   cmdend              = 0;

        if (consport < 0 || commport < 0) return -1;

        if (InvokeObject(commport, PORT_CREATE, NullPointer) != STATUS_OK) goto cleanup;
        if (InvokeObject(consport, PORT_OPEN, "CONSOLE") != STATUS_OK) goto cleanup;

        Message msg;
        memset(&msg, 0, sizeof(Message));
        msg.header.protocol       = VGACONS_PROTOCOL_V0;
        msg.header.payload_length = 0;
        msg.header.type           = VGACONS_CLAIM_CONSOLE;
        msg.header.reply_handle   = commport;

        Status accept_status = STATUS_BAD;

        /* spin until the console server is freed, as we need to have access to it */
        while (accept_status != STATUS_OK) {
                Message reply;
                if (SendMessage(consport, &msg, sizeof(msg.header)) != STATUS_OK) goto cleanup;

                /* Kernel didn't authorize the message to come through */
                if (ReceiveMessage(commport, &reply) != STATUS_OK) continue;
                accept_status = (Status)reply.payload.raw[0];
        }

        puts("* Welcome to DragonWare!\n");
        puts("* DragonWare is a compact, free software, microkernel-based operating system for PC"
             "platforms. You can view the source code at https://github.com/tseli0s/DragonWare "
             "(And thank you for trying it out!)\n");
        puts("You are currently in the \"DragonWare Command Prompt\". The so-called shell in "
             "most other operating systems. You are running a very early, preview version of "
             "DragonWare, so you can't run other programs yet.\n");
        puts("* Type 'help' below to view some of the available builtin commands, "
             "or exit to exit this process.\n");

        printf("DragonWare >> ");

        while (true) {
                Message m;
                ReceiveMessage(commport, &m);
                switch (m.header.protocol) {
                        /* This is forwarded by the console, not sent directly from the keyboard */
                        case KBD_PROTOCOL_V0:
                                char c = (char)m.payload.raw[0];
                                if (CharacterIsPrintable(c)) {
                                        command[cmdend]     = c;
                                        command[cmdend + 1] = '\0';
                                        cmdend++;
                                } else if (c == '\n') {
                                        /* not empty buffer, the command must be handled */
                                        if (cmdend != 0) {
                                                HandleCommandBuffer(command);
                                                memset(command, 0, sizeof(command));
                                                cmdend = 0;
                                        }
                                        printf("DragonWare >> ");
                                } else if (c == '\b') {
                                        if (cmdend > 0) {
                                        	cmdend--;
						command[cmdend] = '\0';
                                                putchar('\b');
                                        }
                                }
                                break;
                        default:
                                break;
                }
        }
        return 0;
cleanup:
        if (consport >= 0) DeleteObject(consport);
        if (commport >= 0) DeleteObject(commport);
        return 0;
}
