/**********************************************************************
 * FILE: error.c
 * PURPOSE: FatalError() implementation for bootloader purposes
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "error.h"

#include <kstring.h>
#include <power.h>
#include <stdarg.h>

#include "textmode/dbgprint.h"
#include "textmode/tui.h"
#include "textmode/vgatext.h"

[[noreturn, gnu::nonnull(1)]]
void FatalError(const char *msg, ...) {
        __asm__ volatile("cli");
        char    buf[256] = {0};
        va_list ap;
        va_start(ap, msg);
        vsnprintf(buf, sizeof(buf), msg, ap);
        va_end(ap);

        Byte colorattr = VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY, VGATEXT_COLOR_BLACK);
        VGAClearAllText(colorattr);
        VGAPrintStringAt(2, 0, "FATAL ERROR!",
                         VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_RED, VGATEXT_COLOR_BLACK));
        VGAPrintStringAt(2, 2, buf, colorattr);
        VGAPrintStringAt(2, 6,
                         "--> You can now safely reboot the machine. If this "
                         "is a bug, please report! https://github.com/tseli0s/DragonWare/issues",
                         colorattr);

        DebugPrint("FATAL ERROR RAISED -- Description: \"%s\"", buf);
        __asm__ volatile("1: jmp 1b");
        __builtin_unreachable();
}

void RecoverableError(const char *msg, ...) {
        DebugPrint("XXX Raising recoverable error to the user!");
        Bool    exit     = false;
        char    buf[256] = {0};
        va_list ap;
        va_start(ap, msg);
        vsnprintf(buf, sizeof(buf), msg, ap);
        va_end(ap);

        Byte colorattr = VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY, VGATEXT_COLOR_BLACK);
        VGAClearAllText(colorattr);
        VGAPrintStringAt(2, 0, "Error!",
                         VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_RED, VGATEXT_COLOR_BLACK));
        VGAPrintStringAt(2, 2, buf, colorattr);
        VGAPrintStringAt(2, 6, "Press any key to return to the main menu...", colorattr);
        while (!exit) {
                __asm__ volatile("hlt");
        }
        DrawUserInterface();
}
