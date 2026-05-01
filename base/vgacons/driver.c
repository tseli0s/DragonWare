/**********************************************************************
 * FILE: driver.c
 * PURPOSE: VGA text mode driver utilities ported from the kernel and bootloader implementations
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

#include "driver.h"

#include <io.h>
#include <kerneltypes.h>
#include <string.h>

/* Vertical position we are currently at */
static int xpos = 0;

/* Horizontal position we are currently at. */
static int ypos = 0;

/* Last color used for a single character print */
static Byte color_attribute = 0;

/* Pointer to the VGA text mode buffer, the kernel takes care of mapping it. */
static volatile u16 *vgabuffer = VGA_CONSOLE_VIRTUAL_ADDR;

#define VGA_TEXT_MODE_INDEX(x, y) ((Size)((y) * VGA_CONSOLE_WIDTH + (x)))

void VGAClearAllText(Byte colorattr) {
        for (Size i = 0; i < VGA_CONSOLE_WIDTH * VGA_CONSOLE_HEIGHT; i++)
                vgabuffer[i] = VGAGetCharacterWithColor(' ', colorattr);
        xpos = 0;
        ypos = 0;
        VGASetCursorPosition((unsigned int)xpos, (unsigned int)ypos);
}

void VGASetCursorPosition(unsigned int x, unsigned int y) {
        u16 pos = (u16)VGA_TEXT_MODE_INDEX(x, y);
        outb(0x3D4, (Byte)0x0E);
        outb(0x3D5, (Byte)((pos >> 8) & 0xFF));

        outb(0x3D4, (Byte)0x0F);
        outb(0x3D5, (Byte)pos & 0xFF);
}

void VGAPrintCharAt(int x, int y, char c, VGAColor color) {
        if (x < 0 || x >= VGA_CONSOLE_WIDTH) return;
        if (y < 0 || y >= VGA_CONSOLE_HEIGHT) return;

        color_attribute  = (Byte)color;
        Size index       = VGA_TEXT_MODE_INDEX(x, y);
        vgabuffer[index] = VGAGetCharacterWithColor(c, (Byte)color);
}

void VGAPrintCharacter(char c, VGAColor color) {
        switch (c) {
                case '\n':
                        xpos = 0;
                        ypos++;
                        break;
                case '\b':
                        if (xpos > 0)
                                xpos--;
                        else if (ypos > 0) {
                                ypos--;
                                xpos = VGA_CONSOLE_WIDTH - 1;
                        }
                        VGAPrintCharAt(xpos, ypos, ' ', color);
                        break;
                default:
                        VGAPrintCharAt(xpos, ypos, c, color);
                        xpos++;
                        if (xpos >= VGA_CONSOLE_WIDTH) {
                                xpos = 0;
                                ypos++;
                        }
                        break;
        }

        if (ypos >= VGA_CONSOLE_HEIGHT) {
                VGAScrollBuffer(); /* This also implicitly updates the cursor */
                ypos = VGA_CONSOLE_HEIGHT - 1;
        } else
                VGASetCursorPosition((unsigned int)xpos, (unsigned int)ypos);
}

void VGAPrintString(const char *str, VGAColor color) {
        while (*str != '\0') {
                VGAPrintCharacter(*str, color);
                str++;
        }
}

void VGAScrollBuffer() {
        memmove((void *)vgabuffer, (const void *)(vgabuffer + VGA_CONSOLE_WIDTH),
                (VGA_CONSOLE_HEIGHT - 1) * VGA_CONSOLE_WIDTH * sizeof(u16));
        for (Size x = 0; x < VGA_CONSOLE_WIDTH; x++)
                vgabuffer[(VGA_CONSOLE_HEIGHT - 1) * VGA_CONSOLE_WIDTH + x] =
                        VGAGetCharacterWithColor(' ', color_attribute);

        ypos = VGA_CONSOLE_HEIGHT - 1;
        xpos = 0;
        VGASetCursorPosition((unsigned int)xpos, (unsigned int)ypos);
}
