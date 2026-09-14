/**********************************************************************
 * FILE: main.c
 * PURPOSE: Bootloader (proper) entry point
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>
#include <power.h>

#include "cpu/bioscall.h"
#include "cpu/idt86.h"
#include "elfldr/elfloader.h"
#include "error.h"
#include "frame.h"
#include "fs/fs.h"
#include "highmem.h"
#include "init/bootentry.h"
#include "kbd/kbd.h"
#include "kmalloc.h"
#include "mbutils.h"
#include "memdetect.h"
#include "proto/multiboot.h"
#include "proto/vbe.h"
#include "storage/partition.h"
#include "textmode/dbgprint.h"
#include "textmode/tui.h"
#include "textmode/vgatext.h"

#define DEFAULT_KERNEL_PATH "KRNLIA32.SYS"
#define BOOTLOADER_ID       "DragonWare Boot Manager"

static Multiboot          *bootinfo = NullPointer;
static MultibootMMapEntry *mmapaddr = NullPointer;

[[gnu::aligned(16)]]
static VBEInfo     *vbe_info      = NullPointer;
static VBEModeInfo *vbe_mode_info = NullPointer;

[[noreturn]]
extern void _JumpToKernel(void *mbaddr, void *addr);

static void InitVBEInformation(VBEInfo *vbe, VBEModeInfo *mi) {
        vbe_info      = (VBEInfo *)AllocateFrame();
        vbe_mode_info = (VBEModeInfo *)AllocateFrame();

        kzeromem(vbe_info, FRAME_SIZE);
        kzeromem(vbe_mode_info, FRAME_SIZE);

        memcpy(vbe_info, vbe, sizeof(VBEInfo));
        memcpy(vbe_mode_info, mi, sizeof(VBEModeInfo));

        bootinfo->controlinfo = (u32)vbe_info;
        bootinfo->modeinfo    = (u32)vbe_mode_info;
}

static void LoadBootModules(const char *volume, const char **module_list, Size list_size) {
        VGAPrintCenteredString(VGA_HEIGHT - 1, "Loading early system services...",
                               DEFAULT_VGA_COLOR);
        MultibootModule *mod_headers = AllocateHighMemory(1);
        if (!mod_headers) goto oom;

        bootinfo->mods_addr = (u32)mod_headers;
        bootinfo->flags |= MULTIBOOT_MODS;

        for (unsigned int i = 0; i < list_size; i++) {
                char buf[512];
                snprintf(buf, sizeof(buf), "Loading server module %s...", module_list[i]);
                VGAPrintCenteredString(VGA_HEIGHT - 1, buf, DEFAULT_VGA_COLOR);
                File f = OpenFile(volume, module_list[i]);
                if (!f.loaded) {
                        RecoverableError(
                                "Can't load server %s. The operating system's functionality is "
                                "going to be limited.",
                                module_list[i]);
                        continue;
                }
                Size  frames_needed = alignup(f.filesize, FRAME_SIZE) / FRAME_SIZE;
                Byte *start         = AllocateHighMemory(frames_needed);
                if (!start) goto oom;

                kzeromem(start, FRAME_SIZE * frames_needed);
                if (ReadFromFile(&f, start, f.filesize) != f.filesize) {
                        RecoverableError(
                                "Can't read whole file into memory for some reason. Is this file "
                                "actually valid?");
                }

                mod_headers[bootinfo->mods_count].start   = (u32)start;
                mod_headers[bootinfo->mods_count].end     = ((u32)start) + f.filesize;
                mod_headers[bootinfo->mods_count].cmdline = 0x00;

                /* spec technically requires this field to be zeroed out although it doesn't
                 * actually matter all that much and we could omit this line */
                kzeromem(&mod_headers[bootinfo->mods_count].padding,
                         sizeof(mod_headers[bootinfo->mods_count].padding));
                bootinfo->mods_count++;
        }
        return;
oom:
        FatalError(
                "Out of memory (Cannot allocate enough pages for the module's information/file "
                "contents)");
}

static void LoadAndBootKernel(const char *volume, Bool fbmode) {
        bootinfo->flags    = MULTIBOOT_MMAP | MULTIBOOT_INFO_BOOTDEV | MULTIBOOT_FRAMEBUFFER_INFO;
        bootinfo->fbtype   = 2; /* Text mode */
        bootinfo->fbwidth  = VGA_WIDTH;
        bootinfo->fbheight = VGA_HEIGHT;
        bootinfo->fbpitch  = 0;
        bootinfo->fbaddr   = VGA_ADDR;
        VGAClearAllText(DEFAULT_VGA_COLOR);

        VBEInfo vbe;
        if (fbmode) {
                bootinfo->flags |= MULTIBOOT_VBEINFO;
                bootinfo->fbtype = 1; /* Linear non-indexed framebuffer */
                Status vbestatus = GetVESAInformationBlock(&vbe);
                if (vbestatus != STATUS_OK) {
                        FatalError(
                                "GetVESAInformationBlock() failed (status %d). Your graphics card "
                                "may not be supported. Please reboot your machine and select text "
                                "mode boot instead.");
                }
                /* The version field has the major version in the high byte and the minor
                 * version in the low byte. */
                u8 vmajor = (vbe.version >> 8) & 0xFF;
                u8 vminor = vbe.version & 0xFF;

                /*
                 * These variables assume real mode addressing (We're talking about a
                 * specification from the DOS days, after all). To access them from
                 * protected mode, we have to convert them to linear addresses.
                 */
                char *vendor   = (char *)SegmentedToLinearAddress((u32)vbe.vendorname);
                char *product  = (char *)SegmentedToLinearAddress((u32)vbe.productname);
                char *revision = (char *)SegmentedToLinearAddress((u32)vbe.productrevision);
                DebugPrint(
                        "VESA video card information: Implementation version %d.%d, Vendor "
                        "%s, "
                        "Product %s "
                        "(Revision %s), video memory: %d kilobytes",
                        vmajor, vminor, vendor, product, revision,
                        vbe.totalmem * 64 /* Because memory in VESA is counted in 64KB blocks */
                );
        }

        VGAPrintCenteredString(VGA_HEIGHT - 1, "Loading DragonWare microkernel...",
                               DEFAULT_VGA_COLOR);
        File f = OpenFile(volume, DEFAULT_KERNEL_PATH);
        if (!f.loaded)
                FatalError(
                        "%s not found in the volume \"%s\"! Check that the filesystem contains the "
                        "file, "
                        "that the media is not failing and that it is detected by the bootloader. "
                        "If you have multiple CD-ROMs connected, try disconnecting them.",
                        DEFAULT_KERNEL_PATH, volume);

        VGAPrintCenteredString(VGA_HEIGHT - 1, "Reading boot protocol header...",
                               DEFAULT_VGA_COLOR);
        u8 buffer[MULTIBOOT_SEARCH_FOR];
        ZeroMemory(buffer);

        Size result = ReadFromFile(&f, buffer, sizeof(buffer));
        if (result < MULTIBOOT_SEARCH_FOR) {
                FatalError("Cannot read %s into memory! Read %d bytes before failure.",
                           DEFAULT_KERNEL_PATH, result);
        }

        const char *modules_needed[] = {
                "ps2kbd.run",
                "vgacons.run",
                "idedrv.run",
                "dcp.run",
        };
        off_t multiboot_addr = FindMultibootHeader(buffer);
        if (multiboot_addr < 0) FatalError("Unable to find Multiboot checksum!");
        DebugPrint("Multiboot header at %d offset within %p", multiboot_addr, buffer);

        MultibootHeader *header = (MultibootHeader *)(buffer + multiboot_addr);
        DebugPrint(
                "Multiboot header: Entry point %p, preferred mode %d, preferred "
                "dimensions %dx%d, at depth %d",
                header->entry, header->mode, header->width, header->height, header->depth);

        if (fbmode) {
                u16         mode = 0;
                VBEModeInfo mi;
                if (FindBestVESAMode(&vbe, &mi, header->width, header->height, header->depth,
                                     &mode) == STATUS_BAD) {
                        FatalError(
                                "FindBestVESAMode() failed unexpectedly. Please reboot and, in the "
                                "menu, select text mode boot to see if the problem persists. If "
                                "you recently updated your graphics hardware, check with your "
                                "vendor to verify that the VESA BIOS extensions are available.");
                }
                if (VESAModeset(mode) != STATUS_OK) {
                        FatalError(
                                "Unable to switch to selected video mode. Please select another "
                                "boot option.");
                } else {
                        bootinfo->fbwidth  = mi.width;
                        bootinfo->fbheight = mi.height;
                        bootinfo->bpp      = mi.bpp;
                        bootinfo->fbaddr   = mi.framebuffer;
                        bootinfo->fbpitch  = mi.pitch;
                        InitVBEInformation(&vbe, &mi);

                        /* Normally this is the VGA text mode driver, but since we are booting in
                         * graphical mode, we need to switch the driver. */
                        modules_needed[1] = "fbsrv.run";
                }
        }

        uintptr_t entry = 0;
        ReadELFToMemory(&f, &entry);

        LoadBootModules(volume, modules_needed, arraysize(modules_needed));

        /* Unnecessary, but let's not leave remnants of the bootloader before entering the kernel */
        if (!fbmode) VGAClearAllText(DEFAULT_VGA_COLOR);

        /* We are entering the kernel! */
        _JumpToKernel(bootinfo, (void *)entry);
}

static void BootDragonWareFromCDText(void) {
        DebugPrint("Booting from CD volume in text mode...");
        LoadAndBootKernel("cd0", false);
}

static void BootDragonWareFromCDGraphical(void) {
        DebugPrint("Booting from CD volume in graphical mode...");
        LoadAndBootKernel("cd0", true);
}

static void BootDragonWareDefaultOptions(void) {
        DebugPrint("Booting from first available partition in graphical mode...");
        LoadAndBootKernel("hd0/p0", true);
}

static void BootDragonWareVGATextMode(void) {
        DebugPrint("Booting from first available partition in text mode...");
        LoadAndBootKernel("hd0/p0", false);
}

static void CopyMemoryRegionsToMultibootStruct(void) {
        Size              n       = 0;
        MemoryRegionE820 *regions = FetchMemoryRegions(&n);
        for (Size i = 0; i < n; i++) {
                mmapaddr[i].addr = regions[i].base;
                mmapaddr[i].len  = regions[i].length;
                mmapaddr[i].type = regions[i].type;
                mmapaddr[i].size = 20; /* We only support the core 20 byte entry model */
        }
        bootinfo->mmap_len = n * sizeof(MultibootMMapEntry);
}

static void InitMultibootStructure(Byte BootDevice) {
        /* We need a page-aligned address here. */
        bootinfo = (Multiboot *)AllocateFrame();
        kzeromem(bootinfo, FRAME_SIZE);

        /* Not here, but good idea to have it anyways */
        mmapaddr = (MultibootMMapEntry *)AllocateFrame();
        kzeromem(mmapaddr, FRAME_SIZE);

        /* NEVER free this */
        Size  bootidlen         = strlen(BOOTLOADER_ID) + 1;
        char *bootloader_vendor = kmalloc(bootidlen);
        if (!bootloader_vendor) FatalError("Out of memory!");

        memcpy(bootloader_vendor, BOOTLOADER_ID, bootidlen);
        bootloader_vendor[bootidlen - 1] = '\0';
        bootinfo->boot_device            = (u32)BootDevice;
        bootinfo->bootloader             = (u32)bootloader_vendor;
        bootinfo->mmap_addr              = (u32)mmapaddr;
        CopyMemoryRegionsToMultibootStruct();
}

[[gnu::noreturn]]
void bootmain(void) {
        InitDebugPrint();
        DebugPrint("Welcome to DragonWare Boot Manager!");

        extern Byte BootDevice;
        extern Word NumMemoryRegions;

        DebugPrint("Boot device reported from the BIOS to be 0x%x", BootDevice);
        DebugPrint("%d memory regions in this machine.", NumMemoryRegions);

        IDTInit();
        InitPartitionTable();
        VGATextInit();
        InitPS2Keyboard();
        FetchMemoryRegions(NullPointer);
        InitFrameManager();
        AllocHighInit();

        InitMultibootStructure(BootDevice);
        /*
         * Booting from CD
         * TODO: Have a better check here I don't think this'll do
         * */
        if (BootDevice >= 0xE0) {
                AddEntry("Boot DragonWare (CD-ROM, text mode)", 0, BootDragonWareFromCDText);
                AddEntry("Boot DragonWare (CD-ROM, video mode)", 1, BootDragonWareFromCDGraphical);
                AddEntry("Boot DragonWare (Default hard drive, video mode)", 2,
                         BootDragonWareDefaultOptions);
        } else {
                /* TODO */
                AddEntry("Boot DragonWare (Default hard drive, text mode)", 0,
                         BootDragonWareVGATextMode);
                AddEntry("Boot from non-primary drive... (TODO)", 1, NullPointer);
                AddEntry("Boot DragonWare (Default hard drive, video mode)", 2,
                         BootDragonWareDefaultOptions);
        }
        AddEntry("Reboot", 3, ForceReboot);

        DrawUserInterface();
        __asm__ volatile("sti");

        while (1) {
                __asm__ volatile("hlt");
        }
}
