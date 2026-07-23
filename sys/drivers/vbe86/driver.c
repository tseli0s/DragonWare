/**********************************************************************
 * FILE: driver.c
 * PURPOSE: Generic VESA BIOS Extensions driver implementation
 * PROJECT: DragonWare Kernel Drivers
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#define FB_MMIO_FLAGS (PAGE_PRESENT | PAGE_RW | PAGE_CACHE_DISABLED | PAGE_WRITETHROUGH)

#include <atomic.h>
#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <mmutils.h>
#include <panic.h>

#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "drivers/driversdk.h"
#include "fbstate.h"
#include "fonts/glyph.h"
#include "init/bootinfo.h"
#include "iomgr/class.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "vbe.h"
#include "vendor/multiboot.h"
#include "video/pixels.h"

typedef struct [[gnu::packed]] _VESAVersion {
        u8 major, minor;
} VESAVersion;

/* This is almost identical to VBEInfo, but some unused parts have been omitted, so that we can put
 * it directly into the driver data without needing to allocate huge amounts of memory. */
typedef struct _FramebufferControllerInformation {
        VESAVersion version;
        char       *oem;
        u32         capabilities;
        u32         totalmem;
        u16         revision;
        char       *vendorname;
        char       *productname;
        char       *productrevision;
} FramebufferControllerInformation;

/* Only used for VBE pointers which come from real mode. Taken from bootmanager/core/cpu/bioscall.h
 * and adapted somewhat to this file's needs. */
static inline void *SegmentedToLinearPointer(void *seg) {
        u32 addr = (u32)seg;
        u16 hi   = (u16)((addr >> 16) & 0xFFFF);
        u16 lo   = (u16)(addr & 0xFFFF);
        return (void *)((hi * 16) + lo);
}

[[gnu::hot]]
static void WriteSinglePixel(void *privatedata, Size x, Size y, PixelColor color) {
        FramebufferState *state = privatedata;
        switch (state->bpp) {
                case 32: {
                        u32 colorbytes = ColorToBytes32(color);

                        PutPixel32(state->addr, state->width, x, y, colorbytes);
                        break;
                }
                case 24: {
                        u32 colorbytes = ColorToBytes24(color);
                        PutPixel24(state->addr, state->pitch, x, y, colorbytes);
                        break;
                }

                default:
                        LogMessage(LOG_WARNING,
                                   "Unknown framebuffer format. Assuming 32 bits per pixel.");
                        PutPixel32(state->addr, state->width, x, y, ColorToBytes32(color));
                        break;
        }
}

[[gnu::hot]]
static void RenderGlyph(FramebufferState *state, Size x, Size y, Glyph *g) {
        u8 *glyph = g->font + g->offset;

        for (u32 row = 0; row < FONT_HEIGHT; row++) {
                u8 bits = glyph[row];

                for (u32 col = 0; col < FONT_WIDTH; col++) {
                        Size px = x + col;
                        Size py = y + row;

                        if (unlikely(px >= state->width || py >= state->height)) continue;

                        PixelColor color = (bits & (0x80 >> col)) ? state->fg : state->bg;
                        WriteSinglePixel(state, px, py, color);
                }
        }
}
static void DrawRectangle(void *privatedata, Size x, Size y, Size w, Size h, PixelColor color) {
        FramebufferState *state = privatedata;
        for (Size sx = x; sx < w; sx++) {
                for (Size sy = y; sy < h; sy++) {
                        if (state->bpp == 32)
                                PutPixel32(state->addr, state->width, sx, sy,
                                           ColorToBytes32(color));
                        else if (state->bpp == 24)
                                PutPixel24(state->addr, state->pitch, sx, sy,
                                           ColorToBytes24(color));
                }
        }
}

/* In linear framebuffers with direct pixel plotting, we don't need that. */
static void FlushFramebuffer(void *privatedata) { UnusedParameter(privatedata); }

[[gnu::hot]]
static inline void WriteSingleCharacterAt(FramebufferState *state, Size x, Size y, char c) {
        const Glyph g = GetGlyphFromDefaultFont(c);
        RenderGlyph(state, x * FONT_WIDTH, y * FONT_HEIGHT, (Glyph *)&g);
}

[[gnu::hot]]
static void ScrollFramebuffer(FramebufferState *state) {
        /* No I don't understand this either it's 1AM and I just wanna have a cig and go to sleep it
         * just works so I'll do that
         *
         * Btw for some reason memmove_atomic is faster I have absolutely no fucking idea why that
         * is, maybe I need to use vector operations in regular memcpy :P
         */
        memmove_atomic(state->addr, (u8 *)state->addr + (FONT_HEIGHT * state->pitch),
                       (state->height - FONT_HEIGHT) * state->pitch);

        for (Size i = 0; i < state->width / FONT_WIDTH; i++)
                WriteSingleCharacterAt(state, i, (state->height / FONT_HEIGHT) - 1, ' ');

        state->row    = (state->height / FONT_HEIGHT) - 1;
        state->column = 0;
}

[[gnu::hot]]
static void WriteSingleCharacter(void *privatedata, char c) {
        FramebufferState *state    = privatedata;
        const Size        max_cols = (state->width / FONT_WIDTH) - 2;

        if (c == '\n') {
                state->column = 0;
                state->row++;
                if (state->row >= (state->height / FONT_HEIGHT) - 1) ScrollFramebuffer(state);
                return;
        } else if (c == '\t') {
                state->column = (state->column + 4) & (Size)~3;
                return;
        }
        WriteSingleCharacterAt(state, state->column, state->row, c);
        state->column++;
        if (state->column >= max_cols) {
                state->column = 0;
                state->row++;
        }
}

FramebufferInformation GetFramebufferInfo(void *privatedata) {
        FramebufferState      *state = privatedata;
        FramebufferInformation info  = {
                 .width  = state->width,
                 .height = state->height,
                 .bpp    = state->bpp,
                 .stride = state->pitch /* i believe stride and pitch are the same i dont remember
                                      honestly */
        };
        return info;
}

static void DeleteSingleCharacter(void *privatedata) {
        FramebufferState *state = privatedata;
        if (state->column != 0) state->column--;
        WriteSingleCharacterAt(state, state->column, state->row, ' ');
}

static inline void SetColors(void *privatedata, PixelColor fg, PixelColor bg) {
        FramebufferState *state = privatedata;
        state->fg               = fg;
        state->bg               = bg;
}

static void ClearFramebuffer(void *privatedata) {
        FramebufferState *state = privatedata;
        state->column           = 0;
        state->row              = 0;
        switch (state->bpp) {
                case 32: {
                        u32 color = ColorToBytes32(state->bg);
                        for (Size y = 0; y < state->height; y++) {
                                for (Size x = 0; x < state->width; x++)
                                        PutPixel32(state->addr, state->width, x, y, color);
                        }
                        break;
                }
                case 24: {
                        u32 color = ColorToBytes24(state->bg);
                        for (Size y = 0; y < state->height; y++) {
                                for (Size x = 0; x < state->width; x++)
                                        PutPixel24(state->addr, state->pitch, x, y, color);
                        }
                        break;
                }
                default:
                        LogMessage(LOG_DEBUG, "Unknown framebuffer format, not clearing");
                        break;
        }
}

static void ResetFramebufferConsole(void *privatedata) {
        FramebufferState *state = privatedata;
        state->row              = 0;
        state->column           = 0;
        ClearFramebuffer(privatedata);
}

static void SetConsoleColorAttributes(void *privatedata, PixelColor bg, PixelColor fg) {
        SetColors(privatedata, fg, bg);
}

/* TODO: Mode information storing too */
[[gnu::nonnull]]
static void CollectVBEInformation(Multiboot *bootinfo, FramebufferControllerInformation *ctrl) {
        /* This structure is in low memory, and therefore needs to be mapped to be accessible. Also,
         * many pointers here are segmented pointers, so we have to convert them (look at the
         * bootloader source to get a glimpse) */
        if (!bootinfo->controlinfo) goto bad;

        uintptr_t infoaddr  = aligndown(bootinfo->controlinfo, PAGE_SIZE);
        Status    mapstatus = MapSinglePage(infoaddr, infoaddr, PAGE_PRESENT);
        if (mapstatus != STATUS_OK) {
                LogMessage(LOG_WARNING,
                           "%s: Mapping controller information (Address %p) failed: %s", __func__,
                           infoaddr, StatusCodeToString(mapstatus));
                goto bad;
        }

        VBEInfo *info         = (VBEInfo *)bootinfo->controlinfo;
        ctrl->oem             = SegmentedToLinearPointer(info->oem);
        ctrl->productname     = SegmentedToLinearPointer(info->productname);
        ctrl->productrevision = SegmentedToLinearPointer(info->productrevision);
        ctrl->vendorname      = SegmentedToLinearPointer(info->vendorname);
        ctrl->version         = (VESAVersion){.major = (u8)((info->version >> 8) & 0xFF),
                                              .minor = (u8)(info->version & 0xFF)};
        ctrl->revision        = info->revision;
        ctrl->totalmem        = info->totalmem * 64;
        ctrl->capabilities    = info->capabilities;

        UnmapSinglePage(infoaddr);
        return;
bad:
        kzeromem(ctrl, sizeof(FramebufferControllerInformation));
        LogMessage(LOG_WARNING, "Error during %s(), zeroing out FramebufferControllerInformation",
                   __func__);
}

Status VBE86DriverInit(void) {
        Multiboot *bootinfo = GetBootInformationStructure();
        if (!bootinfo) return STATUS_NOT_FOUND;
        LogMessage(LOG_INFO, "Initializing VESA-based framebuffer driver");

        /* Per multiboot spec, type = 2 means VGA text mode */
        if (bootinfo->fbtype == 2) return STATUS_UNSUPPORTED;

        DeviceManagerNode *fb = MakeDeviceNode("Kernel Framebuffer", P_MUTABLE | P_DIRECT_ACCESS,
                                               DEVCLASS_FRAMEBUFFER | DEVCLASS_CONSOLE);
        if (!fb) return STATUS_OUT_OF_MEMORY;

        fb->devtable.ddo   = kzalloc(sizeof(DeviceOperations));
        fb->attr.mmio_addr = bootinfo->fbaddr;
        if (bootinfo->fbpitch > 0)
                fb->attr.mmio_len = bootinfo->fbpitch * bootinfo->fbheight;
        else
                fb->attr.mmio_len = (bootinfo->bpp / 8) * bootinfo->fbwidth * bootinfo->fbheight;

        if (!fb->devtable.ddo) {
                kfree(fb);
                return STATUS_OUT_OF_MEMORY;
        }

        LogMessage(LOG_INFO, "Dumping framebuffer information from bootloader protocol:");
        LogMessage(LOG_INFO, "\tDimensions: %dx%d pixels at %d bytes per pixel", bootinfo->fbwidth,
                   bootinfo->fbheight, bootinfo->bpp);
        LogMessage(LOG_INFO, "\tPitch: %d bytes", bootinfo->fbpitch);
        LogMessage(LOG_INFO, "\tPhysical MMIO Address: %p", (uintptr_t)bootinfo->fbaddr);

        /* I'm just praying that we haven't overwritten this data so far. Very unlikely, but could
         * happen, honestly. FIXME? */
        if (bootinfo->flags & MULTIBOOT_VBEINFO) {
                FramebufferControllerInformation ci = {0};
                CollectVBEInformation(bootinfo, &ci);
                {
                        uintptr_t p_vendor   = aligndown((uintptr_t)ci.vendorname, PAGE_SIZE);
                        uintptr_t p_product  = aligndown((uintptr_t)ci.productname, PAGE_SIZE);
                        uintptr_t p_revision = aligndown((uintptr_t)ci.productrevision, PAGE_SIZE);
                        if (p_vendor) MapSinglePage(p_vendor, p_vendor, PAGE_PRESENT);
                        if (p_product) MapSinglePage(p_product, p_product, PAGE_PRESENT);
                        if (p_revision) MapSinglePage(p_revision, p_revision, PAGE_PRESENT);

                        LogMessage(LOG_INFO,
                                   "Dumping VESA-compatible controller information in use by the "
                                   "kernel:");
                        LogMessage(LOG_INFO, "\tVendor: %s, Product: %s, Revision %s",
                                   ci.vendorname, ci.productname, ci.productrevision);
                        LogMessage(LOG_INFO, "\tImplementation version %d.%d (rev. %d)",
                                   ci.version.major, ci.version.minor, ci.revision);
                        LogMessage(LOG_INFO, "\tTotal video memory: %d kilobytes", ci.totalmem);

                        UnmapSinglePage(p_vendor);
                        UnmapSinglePage(p_product);
                        UnmapSinglePage(p_revision);
                }
        }

        FramebufferDeviceOps fbddo = {
                .WriteSinglePixel          = WriteSinglePixel,
                .BlitRectangle             = DrawRectangle,
                .SetCurrentOutputColors    = SetColors,
                .ClearScreen               = ClearFramebuffer,
                .Flush                     = FlushFramebuffer,
                .GetFramebufferInformation = GetFramebufferInfo,
        };

        ConsoleDeviceOps conddo       = {.WriteSingleChar   = WriteSingleCharacter,
                                         .DeleteSingleChar  = DeleteSingleCharacter,
                                         .ResetConsole      = ResetFramebufferConsole,
                                         .SetTextAttributes = SetConsoleColorAttributes};
        fb->devtable.ddo->framebuffer = fbddo;
        fb->devtable.ddo->console     = conddo;

        if ((uintptr_t)bootinfo->fbaddr >= (0xFFFFFFFF - PAGE_SIZE))
                FatalError("Framebuffer MMIO too high in the address space!");

        Size pages_needed =
                ((Size)bootinfo->fbpitch * bootinfo->fbheight + PAGE_SIZE - 1) / PAGE_SIZE;
        if (pages_needed == 0) { /* we do something wrong */
                LogMessage(LOG_WARNING,
                           "Possible bug: pages_needed set to 0, so no virtual mapping of the "
                           "framebuffer can be done!");
                return STATUS_BAD;
        }

        if (MapMemoryRange((uintptr_t)bootinfo->fbaddr, FRAMEBUFFER_ADDR, FB_MMIO_FLAGS,
                           pages_needed) != pages_needed) {
                LogMessage(LOG_ERROR,
                           "Bad MapMemoryRange() return value; A virtual address couldn't "
                           "be mapped");
                return STATUS_OUT_OF_MEMORY;
        }
        fb->attr.kernel_mapped_addr = FRAMEBUFFER_ADDR;

        fb->private_state = kmalloc(sizeof(FramebufferState));
        if (!fb->private_state) {
                kfree(fb);
                return STATUS_OUT_OF_MEMORY;
        }
        /* Easier than just casting every time */
        FramebufferState *state = fb->private_state;
        state->addr             = (void *)FRAMEBUFFER_ADDR;
        state->bg               = BlackPixel;
        state->fg               = WhitePixel;
        state->bpp              = bootinfo->bpp;
        state->width            = bootinfo->fbwidth;
        state->height           = bootinfo->fbheight;
        state->pitch            = bootinfo->fbpitch;
        state->pixels_per_char  = FONT_WIDTH * FONT_HEIGHT;
        state->column           = 0;
        state->row              = 0;
        state->text_mode        = true;

        fb->private_state = state;
        AddDevice(GetRootDeviceManagerNode(), fb);
        return STATUS_OK;
}

const DriverDescriptor vbe86_descriptor = {.name         = "VBE Generic Driver (x86)",
                                           .author       = "DragonWare",
                                           .license      = "GPLv3.0",
                                           .init_earlier = false,
                                           .__init       = &VBE86DriverInit,
                                           .__delete     = NullPointer};
ADD_DRIVER_DESCRIPTOR(vbe86_descriptor);
