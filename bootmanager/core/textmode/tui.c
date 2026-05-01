/**********************************************************************
 * FILE: tui.c
 * PURPOSE: Text mode user interface for the bootloader
 * PROJECT: DragonWare Boot Manager
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#define DRAW_SELECT_CHAR_OFFSET_X (5)
#define DRAW_SELECT_CHAR_OFFSET_Y (6)

#include "tui.h"

#include "init/bootentry.h"
#include "vgatext.h"

/* The extra spaces are to occupy the entire first column */
#define HEADER "                             DragonWare Boot Manager                            "

void DrawUserInterface(void) {
        VGAClearAllText(DEFAULT_VGA_COLOR);
        VGAPrintStringAt(0, 0, HEADER,
                         VGAGetColorAttribute(VGATEXT_COLOR_BLACK, VGATEXT_COLOR_LIGHT_GREY));
        VGAPrintStringAt(5, 3, "Please select a boot option:", DEFAULT_VGA_COLOR);
        Size       n_entries = 0;
        BootEntry *entries   = GetAllBootEntries(&n_entries);
        Size       current   = GetCurrentIndex();
        Size       visible_i = 0;

        for (Size i = 0; i < n_entries; i++) {
                if (entries[i].name != NullPointer && entries[i].name[0] &&
                    entries[i].SelectCallback) {
                        Size y = DRAW_SELECT_CHAR_OFFSET_Y + visible_i;

                        Byte colorattr = (current == i)
                                                 ? VGAGetColorAttribute(VGATEXT_COLOR_BLACK,
                                                                        VGATEXT_COLOR_LIGHT_GREY)
                                                 : DEFAULT_VGA_COLOR;

                        for (int j = 0; j < VGA_WIDTH; j++) VGAPrintCharAt(j, y, ' ', colorattr);
                        VGAPrintStringAt(DRAW_SELECT_CHAR_OFFSET_X, y, entries[i].name, colorattr);
                        visible_i++;
                }
        }
        /* Extra spaces are to make wrapping look nicer (because I'm too lazy to add hyphens as
         * breaks instead) */
        VGAPrintStringAt(3, VGA_HEIGHT - 3,
                         "Use the arrow keys to navigate the boot entries,  ENTER to "
                         "boot the selected  entry. If the keyboard does not work, try enabling "
                         "PS/2 emulation in the BIOS.",
                         DEFAULT_VGA_COLOR);
}
