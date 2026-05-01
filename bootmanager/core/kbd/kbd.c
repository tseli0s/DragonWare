/**********************************************************************
 * FILE: kbd.c
 * PURPOSE: PS/2 Keyboard support implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "kbd.h"

#include <iodelay.h>
#include <ioport.h>
#include <ktypes.h>
#include <stdbool.h>

#include "cpu/irq.h"
#include "init/bootentry.h"
#include "macros.h"
#include "textmode/tui.h"

#define PS2_IDENTIFY_KBD     (0xF2)
#define PS2_CTRL_ACK         (0xFA)
#define PS2_CTRL_RESEND      (0xFE)
#define PS2_ENABLE_SCANNING  (0xF4)
#define PS2_DISABLE_SCANNING (0xF5)

#define PS2_PORT_DATA        (0x60)
#define PS2_PORT_STATUS      (0x64)
#define PS2_PORT_WRITE       (0x64)

#define MAX_RESEND_RETRIES   (5)
#define PORT_TIMEOUT         (1900000)

static Bool extended_key = false;
static Size curr_row     = 0;

static const char scancodemap[128] = {
        0,   27,  '1', '2', '3', '4', '5', '6', '7',  '8', '9', '0',  '-',  '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p', '[', ']',  '\n', 0,   'a',  's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',  'x', 'c',  'v',
        'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,    ' ', 0,   0,    0,    0,   0,    0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0};

/* On invalid reads, the hardware may make us spin forever for no reason, so we must also have a
 * timeout here for safety. */
static inline Status WaitWrite(void) {
        u32 timeout = PORT_TIMEOUT;
        while (inb(PS2_PORT_STATUS) & 2) {
                if (--timeout == 0) return STATUS_TIMEOUT;
                IODelay();
        }
        return STATUS_OK;
}

static inline Status WaitRead(void) {
        u32 timeout = PORT_TIMEOUT;
        while (!(inb(PS2_PORT_STATUS) & 1)) {
                if (--timeout == 0) return STATUS_TIMEOUT;
                IODelay();
        }
        return STATUS_OK;
}

static void SendKeyboardCommand(Byte cmd) {
        static int retries = MAX_RESEND_RETRIES;
        WaitWrite();
        outb(PS2_PORT_DATA, cmd);
        if (WaitRead() == STATUS_OK) {
                Byte response = inb(PS2_PORT_DATA);
                if (response == PS2_CTRL_ACK) {
                        retries = MAX_RESEND_RETRIES;
                        return;
                } else if (response == PS2_CTRL_RESEND) {
                        while (retries > 0) {
                                SendKeyboardCommand(cmd);
                                retries--;
                        }
                }
        }
}

static inline char ScancodeToCharacter(Byte scancode) {
        if (scancode >= 128) return 0;
        if (scancode == SCANCODE_EXTENDED_COMING) {
                extended_key = true;
                return 0;
        }
        return scancodemap[scancode];
}

static void KeyboardIRQHandler(IRQRegisters *r) {
        UnusedParameter(r);

        Byte sc = inb(PS2_PORT_DATA);
        if (sc == SCANCODE_EXTENDED_COMING) {
                extended_key = true;
                return;
        }

        if (sc & 0x80) {
                extended_key = false;
                return;
        }

        if (extended_key) {
                extended_key = false;

                switch (sc) {
                        case SCANCODE_UP_KEY: {
                                Size       n_entries    = 0;
                                BootEntry *entries_list = GetAllBootEntries(&n_entries);

                                /* Search backwards for the previous valid entry to avoid selecting
                                 * empty ones  */
                                if (curr_row > 0) {
                                        Size prev_row = curr_row - 1;
                                        while (prev_row > 0 &&
                                               entries_list[prev_row].name == NullPointer)
                                                prev_row--;
                                        if (entries_list[prev_row].name != NullPointer)
                                                curr_row = prev_row;
                                }
                                break;
                        }
                        case SCANCODE_DOWN_KEY: {
                                Size       n_entries    = 0;
                                BootEntry *entries_list = GetAllBootEntries(&n_entries);

                                /* Skip any intermediate empty entries */
                                Size next_row = curr_row + 1;
                                while (next_row < n_entries) {
                                        if (entries_list[next_row].name != NullPointer) {
                                                curr_row = next_row;
                                                break;
                                        }
                                        next_row++;
                                }
                                break;
                        }
                }
                DrawUserInterface(); /* Redraw needed to update the selected entry's
                                        background/foreground */
                UpdateIndex(curr_row);
                return;
        }
        char c = ScancodeToCharacter(sc);
        if (c == '\n' || c == '\r') {
                EntrySelected();
                return;
        }
}

void InitPS2Keyboard(void) {
        SendKeyboardCommand(PS2_DISABLE_SCANNING);
        SendKeyboardCommand(PS2_ENABLE_SCANNING);
        RegisterIRQHandler(1, KeyboardIRQHandler);
        curr_row = 0;
}
