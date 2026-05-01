/**********************************************************************
 * FILE: textmode.c
 * PURPOSE: VGA colorful text mode console implementation
 * PROJECT: DragonWare Kernel Drivers
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ioport.h>
#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <mmutils.h>

#include "../driversdk.h"
#include "assert.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "init/bootinfo.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "vendor/multiboot.h"

#define VGA_WIDTH     (80)
#define VGA_HEIGHT    (25)
#define VGA_PTR_COLOR (0xB8000)

/* TODO: Support monochrome video (MMIO address 0xB0000)*/

/*
 * The amount of padding to leave between the corner of the screen and the last
 * character, in characters. You can freely redefine this as you wish as long as
 * it's between 0 and VGA_WIDTH.
 */
#define VGA_XPADDING  (2)

/* The size of the VGA text mode buffer. Each character is 16 bits wide,
 * eight bits for the color/attributes and eight for the character. */
#define VGA_BUFSIZE   ((VGA_WIDTH * VGA_HEIGHT) * sizeof(u16))

/**
 * @brief Colors that may be used in VGA text mode for a single character.
 */
typedef enum _VGAColor {
        VGATEXT_COLOR_BLACK         = 0,
        VGATEXT_COLOR_BLUE          = 1,
        VGATEXT_COLOR_GREEN         = 2,
        VGATEXT_COLOR_CYAN          = 3,
        VGATEXT_COLOR_RED           = 4,
        VGATEXT_COLOR_MAGENTA       = 5,
        VGATEXT_COLOR_BROWN         = 6,
        VGATEXT_COLOR_LIGHT_GREY    = 7,
        VGATEXT_COLOR_DARK_GREY     = 8,
        VGATEXT_COLOR_LIGHT_BLUE    = 9,
        VGATEXT_COLOR_LIGHT_GREEN   = 10,
        VGATEXT_COLOR_LIGHT_CYAN    = 11,
        VGATEXT_COLOR_LIGHT_RED     = 12,
        VGATEXT_COLOR_LIGHT_MAGENTA = 13,
        VGATEXT_COLOR_LIGHT_BROWN   = 14,
        VGATEXT_COLOR_WHITE         = 15,
} VGAColor;

typedef struct _VGATextModeState {
        Bool         init;
        unsigned int row, column;
        u16         *buffer_ptr;
        Byte         colorattr;
} VGATextModeState;

static inline Byte VGAGetColorAttribute(VGAColor fg, VGAColor bg) {
        return (Byte)(fg | bg << (Byte)4);
}

static inline u16 VGAGetCharacterWithColor(char uc, Byte color) {
        return (u16)uc | (u16)((u16)color << 8);
}

static VGAColor VGAMatchColorToKernel(PixelColor p) {
        switch (p) {
                case WhitePixel:
                        return VGATEXT_COLOR_LIGHT_GREY;
                case RedPixel:
                        return VGATEXT_COLOR_RED;
                case OrangePixel:
                        return VGATEXT_COLOR_LIGHT_BROWN; /* There's no orange */
                case GreenPixel:
                        return VGATEXT_COLOR_GREEN;
                case PrussianPixel:
                case BluePixel:
                        return VGATEXT_COLOR_BLUE;
                case LightGrayPixel:
                        return VGATEXT_COLOR_LIGHT_GREY;
                case CyanPixel:
                        return VGATEXT_COLOR_CYAN;
                case MagentaPixel:
                        return VGATEXT_COLOR_LIGHT_MAGENTA;
                case BlackPixel:
                default:
                        return VGATEXT_COLOR_BLACK;
        }
}

void VGASetCursorPosition(unsigned int x, unsigned int y) {
        u16 pos = (u16)(((u16)x) * VGA_WIDTH + y);
        outb(0x3D4, (Byte)0x0E);
        outb(0x3D5, (Byte)((pos >> 8) & 0xFF));

        outb(0x3D4, (Byte)0x0F);
        outb(0x3D5, (Byte)pos & 0xFF);
}

static void VGAScroll(void *private) {
        VGATextModeState *state = private;
        memmove((void *)state->buffer_ptr, (const void *)(state->buffer_ptr + VGA_WIDTH),
                (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(u16));
        for (Size x = 0; x < VGA_WIDTH; x++)
                state->buffer_ptr[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
                        VGAGetCharacterWithColor(' ', state->colorattr);

        state->row    = VGA_HEIGHT - 1;
        state->column = 0;
        VGASetCursorPosition(state->row, state->column);
}

[[gnu::hot]]
static inline void VGAPrintCharacterAt(void *internal, unsigned int x, unsigned int y, char c) {
        VGATextModeState *state = internal;
        if (unlikely(!state->init)) return;

        unsigned int index       = y * VGA_WIDTH + x;
        state->buffer_ptr[index] = VGAGetCharacterWithColor(c, (Byte)state->colorattr);
        VGASetCursorPosition(y, x + 1);
}

[[gnu::hot]]
static void VGAPrintCharacter(void *internal, char c) {
        VGATextModeState *state = internal;
        if (!state->init) return;

        if (c == '\n') {
                state->column = 0;
                state->row++;
                if (state->row >= VGA_HEIGHT) VGAScroll(internal);
                VGASetCursorPosition(state->row, state->column);
                return;
        }

        if (c == '\t') {
                state->column += 4;
                if (state->column >= VGA_WIDTH) VGAScroll(internal);
                VGASetCursorPosition(state->row, state->column);
                return;
        }

        if (state->column >= VGA_WIDTH - VGA_XPADDING) {
                state->column = 0;
                state->row++;
                if (state->row >= VGA_HEIGHT) VGAScroll(internal);
        }

        state->column++;
        VGAPrintCharacterAt(internal, state->column, state->row, c);
}

static void VGAClearAllText(void *private_state) {
        VGATextModeState *state = private_state;
        for (Size y = 0; y < VGA_HEIGHT; y++) {
                for (Size x = 0; x < VGA_WIDTH; x++) {
                        const Size index         = y * VGA_WIDTH + x;
                        state->buffer_ptr[index] = VGAGetCharacterWithColor(' ', state->colorattr);
                }
        }
        state->row    = 0;
        state->column = 0;
}

static void VGASetColorMode(void *private_state, PixelColor bg, PixelColor fg) {
        VGATextModeState *state = private_state;
        state->colorattr =
                VGAGetColorAttribute(VGAMatchColorToKernel(fg), VGAMatchColorToKernel(bg));
}

static void VGADeleteSingleCharacter(void *privatedata) {
        VGATextModeState *state = privatedata;
        VGAPrintCharacterAt(privatedata, state->column, state->row, ' ');

        if (state->column != 0) state->column--;

        /* Need to update the cursor again, otherwise it looks stuck (VGAPrintCharacterAt
         * already does it once). Oh yeah, also FIXME: column and row are being confused all
         * over the driver, I need to get some English lessons in the future  */
        VGASetCursorPosition(state->row, state->column + 1);
}

static void VGAEnableCursor(u8 cursor_start, u8 cursor_end) {
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

Status VGATextInit(void) {
        Multiboot *bootinfo = GetBootInformationStructure();
        if (!bootinfo) return STATUS_NOT_FOUND;

        /* Per multiboot spec, type == 2 means that we have EGA like text mode, which is
         * what we want. See also vbe86 driver */
        if (bootinfo->fbtype != 2) return STATUS_UNSUPPORTED;

        /* Extra checks, just to be sure */
        kassert(bootinfo->fbwidth == VGA_WIDTH && bootinfo->fbheight == VGA_HEIGHT);

        DeviceManagerNode *vganode = MakeDeviceNode(
                "VGA Console", P_DIRECT_ACCESS | P_MUTABLE | P_USER, DEVCLASS_CONSOLE);
        if (!vganode) return STATUS_OUT_OF_MEMORY;
        vganode->devtable.ddo = kzalloc(sizeof(DeviceOperations));
        if (!vganode->devtable.ddo) {
                kfree(vganode);
                return STATUS_OUT_OF_MEMORY;
        }

        VGATextModeState *state = kzalloc(sizeof(VGATextModeState));
        if (!state) {
                kfree(vganode->devtable.ddo);
                kfree(vganode);
                return STATUS_OUT_OF_MEMORY;
        }

        MapSinglePage((uintptr_t)bootinfo->fbaddr, FRAMEBUFFER_ADDR,
                      PAGE_PRESENT | PAGE_RW | PAGE_CACHE_DISABLED | PAGE_WRITETHROUGH);

        state->buffer_ptr = (u16 *)FRAMEBUFFER_ADDR;
        state->colorattr  = VGAGetColorAttribute(VGATEXT_COLOR_LIGHT_GREY, VGATEXT_COLOR_BLACK);
        state->column     = 0;
        state->row        = 0;
        state->init       = true;

        ConsoleDeviceOps console_operations = {.WriteSingleChar   = VGAPrintCharacter,
                                               .ResetConsole      = VGAClearAllText,
                                               .DeleteSingleChar  = VGADeleteSingleCharacter,
                                               .SetTextAttributes = VGASetColorMode};
        vganode->devtable.ddo->console      = console_operations;
        vganode->private_state              = state;
        vganode->attr.kernel_mapped_addr    = FRAMEBUFFER_ADDR;
        vganode->attr.mmio_addr             = bootinfo->fbaddr;
        vganode->attr.mmio_len              = (VGA_WIDTH * VGA_HEIGHT) * sizeof(u16);

        /* The bootloader may disable it. DragonWare Boot Manager sure does. We need it, so
         * make sure we reenable it. */
        VGAEnableCursor(14, 15);
        VGAClearAllText(state);
        AddDevice(NullPointer, vganode);
        return STATUS_OK;
}

const DriverDescriptor vgatext_descriptor = {.name         = "BIOS VGA Text Mode Driver",
                                             .author       = "DragonWare",
                                             .license      = "GPLv3.0",
                                             .init_earlier = true,
                                             .__init       = &VGATextInit,
                                             .__delete     = NullPointer};
ADD_DRIVER_DESCRIPTOR(vgatext_descriptor);
