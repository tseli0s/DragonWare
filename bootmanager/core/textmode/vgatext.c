/**********************************************************************
 * FILE: vgatext.c
 * PURPOSE: VGA text mode implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "vgatext.h"

#include <ioport.h>
#include <kstring.h>
#include <ktypes.h>

static volatile u16 *vgamemory = (u16 *)VGA_ADDR;

static inline u16 VGAGetCharacterWithColor(char uc, Byte color) {
        return (u16)uc | (u16)((u16)color << 8);
}

void VGAClearAllText(Byte colorattr) {
        for (int y = 0; y < VGA_HEIGHT; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                        int index        = y * VGA_WIDTH + x;
                        vgamemory[index] = VGAGetCharacterWithColor(' ', colorattr);
                }
        }
}

/* We don't want a cursor at all. */
static inline void DisableVGACursor() {
        /* See https://wiki.osdev.org/Text_Mode_Cursor */
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
}

void VGATextInit(void) {
        VGAClearAllText(VGAGetColorAttribute(VGATEXT_COLOR_DARK_GREY, VGATEXT_COLOR_BLACK));
        DisableVGACursor();
}

void VGAPrintCharAt(int x, int y, char c, VGAColor color) {
        if (c == '\n' || c == '\b') return;

        /*
         * Not a normal character (eg smiley face)
         * To detect any bugs, we'll just print a question mark.
         */
        if ((unsigned char)c < 0x20) c = '?';
        unsigned int index = y * VGA_WIDTH + x;
        vgamemory[index]   = VGAGetCharacterWithColor(c, (Byte)color);
}

void VGAPrintCenteredString(int y, const char *str, VGAColor color) {
        /* Better to be sure no leftover data is there */
        for (int i = 0; i < VGA_WIDTH; i++) VGAPrintCharAt(i, y, ' ', color);
        int x = (VGA_WIDTH / 2) - (strlen(str) / 2);
        if (strlen(str) > VGA_WIDTH) x = 0;

        VGAPrintStringAt(x, y, str, color);
}

void VGAPrintStringAt(int x, int y, const char *str, VGAColor color) {
        int i = 0;
        while (str[i] != '\0') {
                VGAPrintCharAt(x + i, y, str[i], color);
                i++;
        }
}
