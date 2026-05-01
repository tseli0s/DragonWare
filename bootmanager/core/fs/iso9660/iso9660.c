/**********************************************************************
 * FILE: iso9660.c
 * PURPOSE: ISO9660 filesystem driver implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "iso9660.h"

#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "error.h"
#include "kmalloc.h"
#include "storage/ata.h"
#include "storage/partition.h"
#include "textmode/dbgprint.h"

#define SECTOR_SIZE          (512)
#define CD_SECTOR_SIZE       (4 * SECTOR_SIZE)
#define SECTORS_IN_CD_SECTOR (CD_SECTOR_SIZE / SECTOR_SIZE)
#define DESCRIPTOR_ADDR_LBA  (16)
#define MAX_READ_RETRIES     (3)

/* Bits 5 and 6 are reserved, hence the sudden jump */
#define ENTRY_HIDDEN         (0x01)
#define ENTRY_DIRECTORY      (0x02)
#define ENTRY_ASSOCIATED     (0x04)
#define ENTRY_FORMAT_IN_EA   (0x08)
#define ENTRY_PERMISSIONS    (0x10)
#define ENTRY_HUGE_FILE      (0x80)

/* ISO9660 is biendian, so we must have an integer type for biendian integers here. See
 * https://wiki.osdev.org/ISO_9660#Numerical_formats for more details.
 * _LB means Little endian followed by Big endian. */
typedef struct [[gnu::packed]] _u16_LB {
        u16 le;
        u16 be;
} u16_LB;

typedef struct [[gnu::packed]] _i16_LB {
        i16 le;
        i16 be;
} i16_LB;

typedef struct [[gnu::packed]] _u32_LB {
        u32 le;
        u32 be;
} u32_LB;

typedef struct [[gnu::packed]] _i32_LB {
        u32 le;
        u32 be;
} i32_LB;

typedef struct [[gnu::packed]] _PrimaryVolumeDescriptor {
        Byte   type;    /* Always 1 for the PVD */
        char   id[5];   /* CD001 */
        u8     version; /* Always 1 */
        u8     reserved0;
        char   systemid[32]; /* unused by this driver */
        char   volumeid[32]; /* same case */
        Byte   reserved1[8];
        i32_LB volume_space_size;
        Byte   reserved2[32];
        i16_LB volume_set_size;     /* Ignore this */
        i16_LB volume_sequence_num; /* This too */
        i16_LB block_size;          /* Logical block size */
        i32_LB path_table_size;     /* Ignored. For now. */
        u32    path_table_lba_le;
        u32    path_table_lba_opt_le;
        u32    path_table_lba_be;
        u32    path_table_lba_opt_be;
        Byte   root_directory[34]; /* The root directory. Not an LBA address, an actual directory
                                      record. */

        /* Intentionally ignoring the rest of the fields here. We won't need them anyways. */
} PrimaryVolumeDescriptor;

typedef struct [[gnu::packed]] _ISO9660DirectoryRecord {
        u8     length;          /* length of this directory record */
        u8     ext_attr_length; /* length of extended attribute record (is that a thing?) */
        u32_LB lba;             /* Location of extent (LBA) */
        u32_LB data_length;     /* Size of the file/directory */
        Byte   date[7];         /* We don't use this field */
        u8     flags;           /* Flags. Described better here, can't fit it in here:
                                   https://wiki.osdev.org/ISO_9660 */
        u8     file_unit_size;
        u8     interleave_gap_size;
        u16_LB vol_seq_number;
        u8     file_id_length; /* Length of the filename */
        char   file_id[];      /* The filename (Yes variable length, to give us a headache) */
} ISO9660DirectoryRecord;

/* For internal use only */
typedef struct _DriverFileData {
        u32       starting_lba;
        u32       size;
        Partition volume;
} DriverFileData;

[[gnu::nonnull(1)]]
static Bool IsPrimaryVolumeDescriptor(const Byte *data) {
        const PrimaryVolumeDescriptor *pvd           = (PrimaryVolumeDescriptor *)data;
        Bool                           cd001_matches = memcmp(pvd->id, "CD001", 5) == 0;
        if (pvd->type == 0x1 && pvd->version == 0x1 && cd001_matches)
                return true;
        else
                return false;
}

static Status FindDirectoryEntry(const Partition p, const char *path, File *output,
                                 PrimaryVolumeDescriptor *pvd) {
        if (strchr(path, '/') != NullPointer) {
                /* We currently only support files inside the root directory (That's just me getting
                 * slowly used to the ideas of filesystems :P)*/
                return STATUS_UNSUPPORTED;
        }
        ISO9660DirectoryRecord *root = (ISO9660DirectoryRecord *)pvd->root_directory;

        u32 lba  = root->lba.le * (CD_SECTOR_SIZE / SECTOR_SIZE);
        u32 size = alignup(root->data_length.le, SECTOR_SIZE);

        /* FIXME: This assumes the entire directory will fit in one sector. This works while we're
         * still minimal, but if we put too much data in the boot CD, we're doomed. */
        Byte buf[CD_SECTOR_SIZE];
        ZeroMemory(buf);

        Bool read_success = false;
        for (int retry = 0; retry < MAX_READ_RETRIES && !read_success; retry++) {
                if (ATAReadSectors(p.drive_index, size / SECTOR_SIZE, lba, buf) ==
                    size / SECTOR_SIZE) {
                        read_success = true;
                        break;
                }
        }
        if (!read_success) return STATUS_RETRY;

        Size offset = 0;
        while (offset < root->data_length.le) {
                ISO9660DirectoryRecord *entry     = (ISO9660DirectoryRecord *)(buf + offset);
                Size                    stepped   = entry->length;
                Size                    name_size = entry->file_id_length;

                if (!entry->length) {
                        /* ISO9660 zero-pads the end of a sector. Jump to the next 2048 byte
                         * boundary. */
                        offset = alignup(offset + 1, CD_SECTOR_SIZE);
                        continue;
                }
                if (entry->flags & ENTRY_DIRECTORY) {
                        offset += stepped;
                        continue;
                }

                /* No we don't care if a file is hidden that's proprietary big tech ideology, we are
                 * going to load it and send ENTRY_HIDDEN to the abyss. */
                (void)(ENTRY_HIDDEN);

                /* Hackish, but works for now, so that we don't have to manually patch the path for
                 * comparison with a new allocation and stuff like that. */
                for (Size i = 0; i < name_size; i++) {
                        if (entry->file_id[i] == ';') {
                                name_size = i;
                                break;
                        }
                }

                if (strlen(path) != name_size) {
                        offset += stepped;
                        continue;
                }
                if (strncmp(path, entry->file_id, name_size) != 0) {
                        offset += stepped;
                        continue;
                }

                DriverFileData *internal = kmalloc(sizeof(DriverFileData));
                if (unlikely(!internal)) return STATUS_OUT_OF_MEMORY;

                internal->starting_lba = entry->lba.le;
                internal->size         = entry->data_length.le;
                internal->volume       = p;
                output->internal       = internal;
                output->cursor         = 0;
                output->ended          = false;
                output->loaded         = true;
                output->filesize       = entry->data_length.le;

                return STATUS_OK;
        }
        return STATUS_NOT_FOUND;
}

Status OpenFile_ISO9660(const Partition p, const char *path, File *output) {
        output->t = PART_ISO9660_VOL;

        Byte       current_descriptor[CD_SECTOR_SIZE];
        const int  max_search   = 32;
        const Size read_retries = 3;
        Bool       pvd_found    = false;
        ZeroMemory(current_descriptor);

        /* LBAs for CDs must be multiplied by 4, because the sector sizes are four times larger.
         * In case there's a faulty PVD, have a limit to how much we are going to read before just
         * stopping */
        for (int i = 0; i < max_search && !pvd_found; i++) {
                Bool read_success = false;
                for (unsigned int j = 0; j < read_retries && !read_success; j++) {
                        if (ATAReadSectors(p.drive_index, SECTORS_IN_CD_SECTOR,
                                           (DESCRIPTOR_ADDR_LBA + i) * SECTORS_IN_CD_SECTOR,
                                           current_descriptor) == SECTORS_IN_CD_SECTOR)
                                read_success = true;
                }
                if (!read_success)
                        FatalError(
                                "Failed to read from the media index %d. Please try again later. "
                                "If the media is not detected by the bootloader, try removing it "
                                "and reeinserting it again.",
                                p.drive_index);

                pvd_found = IsPrimaryVolumeDescriptor(current_descriptor);
        }

        if (!pvd_found) {
                DebugPrint("Primary Volume Descriptor not detected in the CD-ROM, ignoring.");
                return STATUS_UNSUPPORTED;
        }
        return FindDirectoryEntry(p, path, output, (PrimaryVolumeDescriptor *)current_descriptor);
}
int ReadFile_ISO9660(File *f, Size n_bytes, Byte *output) {
        if (unlikely(f->ended)) return 0;

        DriverFileData *fdata = (DriverFileData *)f->internal;

        Size available_bytes = fdata->size - f->cursor;
        Size bytes_to_read   = (n_bytes > available_bytes) ? available_bytes : n_bytes;

        Byte tmpbuf[CD_SECTOR_SIZE];
        Size read_so_far = 0;

        while (read_so_far < bytes_to_read) {
                /* Determine which sector does the cursor currently sit at */
                u32  current_cd_sector_offset = f->cursor / CD_SECTOR_SIZE;
                Size offset_in_sector         = f->cursor % CD_SECTOR_SIZE;

                u32 absolute_cd_lba  = fdata->starting_lba + current_cd_sector_offset;
                u32 absolute_ata_lba = absolute_cd_lba * SECTORS_IN_CD_SECTOR;

                if (ATAReadSectors(fdata->volume.drive_index, SECTORS_IN_CD_SECTOR,
                                   absolute_ata_lba, tmpbuf) != SECTORS_IN_CD_SECTOR) {
                        break; /* Read failed, return what we have so far */
                }

                Size bytes_left_in_sector = CD_SECTOR_SIZE - offset_in_sector;
                Size bytes_left_to_read   = bytes_to_read - read_so_far;
                Size chunk = (bytes_left_to_read < bytes_left_in_sector) ? bytes_left_to_read
                                                                         : bytes_left_in_sector;

                memcpy(output + read_so_far, tmpbuf + offset_in_sector, chunk);

                read_so_far += chunk;
                f->cursor += chunk;
        }

        if (f->cursor >= fdata->size) f->ended = true;
        return read_so_far;
}
