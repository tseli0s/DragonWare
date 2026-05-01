/**********************************************************************
 * FILE: multiboot.h
 * PURPOSE: Multiboot protocol structures and definitions
 * PROJECT: DragonWare Boot Manager
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>
#include <macros.h>

/******************************************************************************************************
 * Header implementation of the multiboot types and constants.
 * Rewritten from scratch (instead of using existing headers) solely
 * for coding style reasons.
 *****************************************************************************************************/

/* The amount of Bytes from the start of the file will a bootloader read to find
 * the multiboot header. */
#define MULTIBOOT_SEARCH_FOR       8192
#define MULTIBOOT_HEADER_ALIGN     4

/* Magic numbers. The header one is found in the operating system, the other is passed to eax by the
 * bootloader. */
#define MULTIBOOT_HEADER_MAGIC     (0x1BADB002)
#define MULTIBOOT_LOADER_MAGIC     (0x2BADB002)

/* Flags that should be checked to see if the bootloader provided those items.
 */
#define MULTIBOOT_INFO_MEMORY      0x00000001
#define MULTIBOOT_INFO_BOOTDEV     0x00000002
#define MULTIBOOT_CMDLINE          0x00000004
#define MULTIBOOT_MODS             0x00000008
#define MULTIBOOT_AOUT_SYMS        0x00000010
#define MULTIBOOT_MMAP             0x00000040
#define MULTIBOOT_BOOTLOADER       0x00000200
#define MULTIBOOT_VBEINFO          0x00000800
#define MULTIBOOT_FRAMEBUFFER_INFO 0x00001000

typedef struct _MultibootHeader {
        u32 magic;
        u32 flags;
        u32 checksum;

        /* a.out kludge or something idk just ignore it */
        u32 header;
        u32 load_start;
        u32 load_end;
        u32 bss_end;
        u32 entry;

        /* Video function */
        u32 mode;
        u32 width;
        u32 height;
        u32 depth;
} MultibootHeader;

typedef struct [[gnu::packed]] _Multiboot {
        u32 flags;
        u32 mem_bottom;
        u32 mem_top;
        u32 boot_device;
        u32 cmdline;
        u32 mods_count;
        u32 mods_addr;

        /* There's some a.out stuff here that I omitted from this header. It was
         * 16 Bytes long so I'll just replace it with this. */
        Byte padding_1[16];
        u32  mmap_len;
        u32  mmap_addr;
        /* Unsure about those */
        u32  drives_len;
        u32  drives;
        /* Something about config tables something something we don't care just omit it */
        u32  padding_2;
        /* Bootloader name. For some reason instead of making it a char pointer
         * and making that pointer point to that, we have to cast this manually.
         */
        u32  bootloader;
        /* APM data, we won't be needing it. At least not yet. Modify the header
         * when we do though. */
        u32  padding_3;

        /* VBE stuff */
        u32 controlinfo;
        u32 modeinfo;
        u16 mode;
        u16 padding_4;
        u16 if_offset;
        u16 if_len;

        /* Framebuffer information. Here's the good stuff. */
        u64  fbaddr;
        u32  fbpitch;
        u32  fbwidth;
        u32  fbheight;
        Byte bpp;
        Byte fbtype;

        /* There's supposed to be a union here as well but I didn't see any
         * reason to use its contents so... */
} Multiboot;

typedef struct [[gnu::packed]] _MultibootModule {
        u32  start, end;
        u32  cmdline;    /* We don't expect a command line from modules, but
                            whatever, let it be here. */
        Byte padding[4]; /* Four byte padding is necessary apparently. */
} MultibootModule;

typedef struct [[gnu::packed]] _MultibootMMapEntry {
        u32 size;
        u64 addr;
        u64 len;
        u32 type;
} MultibootMMapEntry;
