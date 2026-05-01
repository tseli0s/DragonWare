/**********************************************************************
 * FILE: elfloader.c
 * PURPOSE: Minimal and fast ELF loading implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "highmem.h"
#define MAGIC_ELF_BYTE      (0x7F)
#define MAGIC_ELF_STRING    ("ELF")
#define ELF_32BIT_MAGIC     (1)
#define ELF_LITTLE_ENDIAN   (1)
#define ELF_ABI_TARGET_SYSV (0)
#define ELF_FILE_EXECUTABLE (2)
#define ELF_ARCH_X86        (0x03)

#include <bits.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "error.h"
#include "fs/fs.h"
#include "textmode/dbgprint.h"

typedef enum _ELFSegmentType {
        SEGMENT_NULL    = 0, /* To be ignored */
        SEGMENT_LOAD    = 1, /* Clear size_mem bytes at paddr, then copy size bytes to paddr */
        SEGMENT_DYNAMIC = 2, /* Dynamic linking. Unsupported */
        SEGMENT_INTERP  = 3, /* "contains a file path to an executable to use as an int..." yeah you
                                get the hang of it just ignore it */
        SEGMENT_NOTE    = 4, /* You'll never guess bro */
} ELFSegmentType;

typedef enum _ELFSegmentFlag {
        ELF_SEGMENT_NOFLAGS    = 0x00,
        ELF_SEGMENT_EXECUTABLE = 0x01, /* <--- We only care about this one, see ReadELFToMemory */
        ELF_SEGMENT_WRITABLE   = 0x02,
        ELF_SEGMENT_READABLE   = 0x04,
} ELFSegmentFlag;

typedef struct [[gnu::packed]] _ELFHeader {
        Byte  magic_0x7f;
        char  magic_ascii[3]; /* Must be 'ELF' */
        Byte  bits;           /* 1 for 32 bit, 2 for 64 bit */
        Byte  endianness;     /* 1 for little endian, 2 for big endian */
        Byte  version;
        Byte  abi; /* Must be 0 for System V ABI */
        Byte  padding[8];
        u16   type;
        u16   isa;     /* Instruction set, must be 0x03 for x86 */
        u32   elfver;  /* Must be 1 */
        u32   entry;   /* Entry point of the program */
        off_t pht_off; /* PHT offset */
        u32   sec_off; /* Section offset, unused by us */
        u32   flags;   /* Ignored on x86 */
        u16   header_size;
        u16   pht_entry_size; /* Size of an entry in the program header table */
        u16   pht_n_entries;  /* Number of entries in >> */
        u16   sec_entry_size; /* Size of an entry in the section header table */
        u16   sec_n_entries;  /* Number of entries >> */
        u16   sec_index;      /* Section index */
} ELFHeader;

typedef struct [[gnu::packed]] _ELFProgramSegment {
        u32 type; /* See ELFSegmentType */
        u32 offset;
        u32 vaddr; /* I don't think we actually use that, the kernel initializes virtual memory
                      manually after all */
        u32 paddr; /* But we do use this */
        u32 size;
        u32 size_mem; /* Must be >= size, the extra memory is zeroed */
        u32 flags;
        u32 alignment; /* I think the linker does that too */
} ELFProgramSegment;

/* Maybe add some sort of extra logging here? */
static Bool IsValidELFFile(ELFHeader header) {
        if (header.magic_0x7f != MAGIC_ELF_BYTE) return false;
        if (memcmp(header.magic_ascii, "ELF", 3) != 0) return false;
        if (header.abi != ELF_ABI_TARGET_SYSV) return false;
        if (header.isa != ELF_ARCH_X86) return false;
        if (header.endianness != ELF_LITTLE_ENDIAN) return false;
        if (header.bits != ELF_32BIT_MAGIC) return false;

        return true;
}

Status ReadELFToMemory(File *file, uintptr_t *entry) {
        file->cursor = 0;
        file->ended  = false;

        ELFHeader header;
        ReadFromFile(file, (void *)&header, sizeof(ELFHeader));
        if (!IsValidELFFile(header)) FatalError("File not in a valid executable format");

        DebugPrint("PHT Offset: %u, Entries: %u, Entry Size: %u", header.pht_off,
                   (u32)header.pht_n_entries, (u32)header.pht_entry_size);
        int       segments_loaded   = 0;
        uintptr_t virt_offset       = 0;
        uintptr_t highest_addr_used = 0;

        for (int i = 0; i < header.pht_n_entries; i++) {
                ELFProgramSegment current_seg;
                file->cursor = header.pht_off + (i * header.pht_entry_size);
                ReadFromFile(file, (void *)&current_seg, sizeof(ELFProgramSegment));

                if (current_seg.type != SEGMENT_LOAD) {
                        DebugPrint("Segment %d is not a loadable segment, skipping", i);
                        continue;
                } else
                        segments_loaded++;

                kzeromem((void *)current_seg.paddr, current_seg.size_mem);

                file->cursor = current_seg.offset;

                /* read is in the past tense here, for readability purposes I'm clarifying it here
                 */
                Size read = ReadFromFile(file, (void *)current_seg.paddr, current_seg.size);
                if (read != current_seg.size) {
                        RecoverableError(
                                "Reading kernel into memory failed (Expected to read %u bytes from "
                                "segment, but read %u before failing)",
                                read, current_seg.size);
                        return STATUS_BAD;
                }

                DebugPrint("Segment %d: virtual address %p, physical address %p, size %d", i,
                           (u64)current_seg.vaddr, (u64)current_seg.paddr, current_seg.size_mem);

                /* Uh, I think this is the way to go about this. It does work for DragonWare, but no
                 * idea about anything else. Basically, read the segment into memory, and if it is
                 * an executable segment, use that virtual offset to find the physical entry point.
                 */
                if (IS_SET(current_seg.flags, ELF_SEGMENT_EXECUTABLE))
                        virt_offset = current_seg.vaddr - current_seg.paddr;

                /* Also track the highest address that the loader used, in case the high memory
                 * allocator tries to use kernel memory by accident. */
                uintptr_t seg_end_paddr = current_seg.paddr + current_seg.size_mem;
                if (seg_end_paddr > highest_addr_used) highest_addr_used = seg_end_paddr;
        }

        if (!segments_loaded)
                FatalError("Unable to properly load kernel into memory, file may be corrupted.");

        *entry = header.entry - virt_offset;
        AllocHighBreakAt(pagealign(highest_addr_used), 0);
        return STATUS_OK;
}
