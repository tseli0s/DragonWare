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
#include "storage/ata.h"
#include "storage/partition.h"
#include "textmode/dbgprint.h"
#include "textmode/tui.h"
#include "textmode/vgatext.h"
#include "timer/pit.h"

#define DEFAULT_KERNEL_PATH "KRNLIA32.SYS"
#define BOOTLOADER_ID       "DragonWare Boot Manager"

static Multiboot          *bootinfo = NullPointer;
static MultibootMMapEntry *mmapaddr = NullPointer;

static int countdown_timer_idx = 0;

[[noreturn]]
extern void _JumpToKernel(void *mbaddr, void *addr);

static void LoadDefaultBootModules(const char *volume) {
        VGAPrintCenteredString(VGA_HEIGHT - 1, "Loading early system services...",
                               DEFAULT_VGA_COLOR);
        /* XXX be wary of the order here the list depends on the element before to function
         * basically. */
        const char *default_modules[] = {"ps2kbd.run", "vgacons.run", "dcp.run"};

        MultibootModule *mod_headers = AllocateHighMemory(1);
        if (!mod_headers) goto oom;

        bootinfo->mods_addr = (u32)mod_headers;
        bootinfo->flags |= MULTIBOOT_MODS;

        for (unsigned int i = 0; i < arraysize(default_modules); i++) {
                File f = OpenFile(volume, default_modules[i]);
                if (!f.loaded) {
                        RecoverableError(
                                "Can't load server %s. The operating system's functionality is "
                                "going to be limited.",
                                default_modules[i]);
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

                char *cmdline = kmalloc(strlen(default_modules[i]) + 1);
                if (!cmdline) goto oom;

                mod_headers[bootinfo->mods_count].start   = (u32)start;
                mod_headers[bootinfo->mods_count].end     = ((u32)start) + f.filesize;
                mod_headers[bootinfo->mods_count].cmdline = (u32)cmdline;

                strncpy(cmdline, default_modules[i], strlen(default_modules[i]) + 1);

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

static void BootDragonWareFromCD(void) {
        RemoveTickCallbackFunction(countdown_timer_idx);

        bootinfo->flags    = MULTIBOOT_MMAP | MULTIBOOT_INFO_BOOTDEV | MULTIBOOT_FRAMEBUFFER_INFO;
        bootinfo->fbtype   = 2; /* Text mode */
        bootinfo->fbwidth  = VGA_WIDTH;
        bootinfo->fbheight = VGA_HEIGHT;
        bootinfo->fbpitch  = 0;
        bootinfo->fbaddr   = VGA_ADDR;

        VGAClearAllText(DEFAULT_VGA_COLOR);
        VGAPrintCenteredString(VGA_HEIGHT - 1, "Loading KRNLIA32.SYS...", DEFAULT_VGA_COLOR);
        File f = OpenFile("cd0", DEFAULT_KERNEL_PATH);
        if (!f.loaded)
                FatalError(
                        "%s not found in the CD-ROM! Check that the filesystem contains the file, "
                        "that the CD-ROM is not failing and that it is detected by the bootloader. "
                        "If you have any other CD-ROMs connected, try disconnecting "
                        "them.",
                        DEFAULT_KERNEL_PATH);

        VGAPrintCenteredString(VGA_HEIGHT - 1, "Reading kernel headers...", DEFAULT_VGA_COLOR);
        u8 buffer[MULTIBOOT_SEARCH_FOR];
        ZeroMemory(buffer);

        Size result = ReadFromFile(&f, buffer, sizeof(buffer));
        if (result < MULTIBOOT_SEARCH_FOR) {
                FatalError("Cannot read %s into memory! Read %d bytes before failure.",
                           DEFAULT_KERNEL_PATH, result);
        }
        off_t multiboot_addr = FindMultibootHeader(buffer);
        if (multiboot_addr == 0x123456) FatalError("Unable to find Multiboot checksum!");
        DebugPrint("Multiboot header at %d offset within %p", multiboot_addr, buffer);

        MultibootHeader *header = (MultibootHeader *)(buffer + multiboot_addr);
        DebugPrint(
                "Multiboot header: Entry point %p, preferred mode %d, preferred "
                "dimensions %dx%d, at depth %d",
                header->entry, header->mode, header->width, header->height, header->depth);

        uintptr_t entry = 0;
        ReadELFToMemory(&f, &entry);
        LoadDefaultBootModules("cd0");

        /* Unnecessary, but let's not leave remnants of the bootloader before entering the kernel */
        VGAClearAllText(DEFAULT_VGA_COLOR);
        _JumpToKernel(bootinfo, (void *)entry);
}

static void BootDragonWareDefaultOptions(void) {
        RecoverableError(
                "DragonWare cannot be booted in video mode for now. Please select text mode "
                "instead in the main menu and try again.");
}

static void BootDragonWareVGATextMode(void) {
        RemoveTickCallbackFunction(countdown_timer_idx);

        bootinfo->flags    = MULTIBOOT_MMAP | MULTIBOOT_INFO_BOOTDEV | MULTIBOOT_FRAMEBUFFER_INFO;
        bootinfo->fbtype   = 2; /* Text mode */
        bootinfo->fbwidth  = VGA_WIDTH;
        bootinfo->fbheight = VGA_HEIGHT;
        bootinfo->fbpitch  = 0;
        bootinfo->fbaddr   = VGA_ADDR;

        VGAClearAllText(DEFAULT_VGA_COLOR);
        VGAPrintCenteredString(VGA_HEIGHT - 1, "Loading boot::/KRNLIA32.SYS...", DEFAULT_VGA_COLOR);
        File f = OpenFile("hd0/p0", DEFAULT_KERNEL_PATH);
        if (!f.loaded)
                FatalError("File boot::/%s not found! What kernel should I load?",
                           DEFAULT_KERNEL_PATH);

        VGAPrintCenteredString(VGA_HEIGHT - 1, "Reading kernel headers...", DEFAULT_VGA_COLOR);
        u8 buffer[MULTIBOOT_SEARCH_FOR];
        ZeroMemory(buffer);

        Size result = ReadFromFile(&f, buffer, sizeof(buffer));
        if (result < MULTIBOOT_SEARCH_FOR) {
                FatalError("Cannot read boot::/%s into memory! Read %d bytes before failure.",
                           DEFAULT_KERNEL_PATH, result);
        }
        off_t multiboot_addr = FindMultibootHeader(buffer);
        if (multiboot_addr == 0x123456) FatalError("Unable to find Multiboot checksum!");
        DebugPrint("Multiboot header at %d offset within %p", multiboot_addr, buffer);

        MultibootHeader *header = (MultibootHeader *)(buffer + multiboot_addr);
        DebugPrint(
                "Multiboot header: Entry point %p, preferred mode %d, preferred "
                "dimensions %dx%d, at depth %d",
                header->entry, header->mode, header->width, header->height, header->depth);

        uintptr_t entry = 0;
        ReadELFToMemory(&f, &entry);
        LoadDefaultBootModules("hd0/p0");

        /* Unnecessary, but let's not leave remnants of the bootloader before entering the kernel */
        VGAClearAllText(DEFAULT_VGA_COLOR);
        _JumpToKernel(bootinfo, (void *)entry);
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

[[gnu::noreturn]]
void bootmain(void) {
        DebugPrint("Welcome to DragonWare Boot Manager!");

        extern Byte BootDevice;
        extern Word NumMemoryRegions;

        DebugPrint("Boot device reported from the BIOS to be 0x%x", BootDevice);
        DebugPrint("%d memory regions in this machine.", NumMemoryRegions);

        IDTInit();
        InitDebugPrint();
        VGATextInit();

        StartPITTimer();
        InitPS2Keyboard();
        FetchMemoryRegions(NullPointer);
        InitFrameManager();
        AllocHighInit();
        ATAIdentifyAllDevices(4);
        InitPartitionTable();

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

        /* Okay we're playing with fire here but this will work for the time being */
        strncpy(bootloader_vendor, BOOTLOADER_ID, bootidlen);
        bootloader_vendor[bootidlen - 1] = '\0';
        bootinfo->boot_device            = (u32)BootDevice;
        bootinfo->bootloader             = (u32)bootloader_vendor;
        bootinfo->cmdline                = 0;
        bootinfo->mmap_addr              = (u32)mmapaddr;
        bootinfo->mods_addr              = 0;
        bootinfo->mods_count             = 0;
        CopyMemoryRegionsToMultibootStruct();

        /*
         * Booting from CD
         * TODO: Have a better check here I don't think this'll do
         * */
        if (BootDevice >= 0xE0) {
                AddEntry("Boot DragonWare (CD-ROM, text mode)", 0, BootDragonWareFromCD);
                AddEntry("Boot DragonWare (CD-ROM, video mode)", 1,
                         BootDragonWareDefaultOptions); /* Also TODO here */
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
        AddEntry("Boot DragonWare (Default hard drive, text mode)", 1, BootDragonWareVGATextMode);
        AddEntry("Reboot", 3, ForceReboot);

        DrawUserInterface();

        while (1) {
                __asm__ volatile("hlt");
        }
}
