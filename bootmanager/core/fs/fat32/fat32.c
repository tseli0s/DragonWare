/**********************************************************************
 * FILE: fat32.c
 * PURPOSE: FAT32 support and implementation (Without LFN)
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

/* Credits for this whole implementation go to https://wiki.osdev.org/FAT */

#include "fs/fat32/fat32.h"

#include <kstring.h>
#include <ktypes.h>
#include <limits.h>
#include <macros.h>
#include <mmutils.h>

#include "kmalloc.h"
#include "storage/ata.h"
#include "storage/partition.h"

#define SECTOR_SIZE       (512)
#define FAT32_EOF         (0x0FFFFFF8)
#define FAT32_BAD_CLUSTER (0x0FFFFFF7)

typedef struct [[gnu::packed]] _BIOSParameterBlock {
        Byte jmp[3]; /* Jump instruction to skip the BPB when executing code. See here:
                           https://wiki.osdev.org/FAT */
        char oem[8]; /* OEM. We MIGHT use it when allowing to select partitions to boot from. */
        u16  bytes_per_sector;    /* 512 in most cases. */
        Byte sectors_per_cluster; /* The formatting tool decides that. */
        u16  reserved_sectors;    /* >> (The boot sector and in our case the bootloader must be
                                         included here) */
        Byte n_fats;           /* Number of FATs in the media. Usually two, but one is fine too. The
                                     partitioning tool decides that. */
        u16  root_dir_entries; /* Number of root directory entries. */
        u16  total_sectors_vol; /* Total sectors in the volume. If zero, see sector_count */
        Byte media_desc_type;   /* Media descriptor type, we ignore it for now. */
        u16  _sectors_per_fat;  /* Ignored in FAT32 */
        u16  sectors_per_track; /* Sectors per track, something to do with disk geometry, I
                                        don't care really */
        u16  n_heads;           /* Heads/Sides on the storage media, again, I don't care */
        u32  lbastart;     /* https://wiki.osdev.org/FAT calls it "hidden sectors", but we all know
                                   we only care where the partition starts */
        u32  sector_count; /* If total_sectors_vol would overflow, this field is used and the
                                   former is set to 0*/
        /* ------------- END OF THE STANDARD BPB, NOW EXTENDED BPB ------------------ */
        u32  sectors_per_fat; /* This field is used over the 16 bit one above in FAT32 */
        u16  flags;           /* Flags for the filesystem */
        u16  version;       /* High byte is major version, low byte is minor, drivers should respect
                                    this field (Yes we absolutely will)*/
        u32  root_cluster;  /* Starting cluster of the root directory. Usually 2 but FAT32 allows
                                    it anywhere */
        u16  sector_fsinfo; /* Where is the FSInfo structure in sectors */
        u16  backup_boot_sector;   /* Where is the backup boot sector. */
        Byte reserved[12];         /* These bytes should be zero. */
        Byte drive_number;         /* BIOS values (0x80 for disk, 0x00 for floppies, etc) */
        Byte ntflags;              /* Flags for Windows NT, reserved otherwise */
        Byte signature;            /* Must be 0x28 or 0x29 */
        u32  serial_vol;           /* Volume serial number, we'll ignore it for now */
        char volume_label[11];     /* Volume label string, padded with spaces */
        char system_identifier[8]; /* Usually FAT32 padded with spaces */
        Byte bootcode[420];        /* Boot sector code, this part we never touch */
        u16  boot_signature;       /* You know, 0xAA55 */
} BIOSParameterBlock;

typedef struct [[gnu::packed]] _FSInfo {
        u32  signature;         /* Must be 0x41615252 */
        Byte reserved[480];     /* What the fuck Microsoft? */
        u32  signature_2;       /* Another signature, must be 0x61417272 */
        u32  last_free_cluster; /* Uh just read the wiki the comment is too long to write */
        u32  start_looking_at;  /* >> */
        Byte reserved2[12];     /* ... */
        u32  boot_signature;    /* Must be 0xAA550000 */
} FSInfo;

typedef struct [[gnu::packed]] _DirectoryEntry {
        /* Most drivers put this in one field, but I think it looks better like this */
        char name[8];
        char extension[3];
        Byte attributes;            /* Attributes. If we detect LFN, idk we're screwed */
        Byte nt_reserved;           /* Reserved by Windows NT. */
        Byte creation_time_seconds; /* We ignore this field. */
        u16  creation_time;         /* This one too. */
        u16  creation_date;         /* Yep you know. */
        u16  last_access;           /* Same. */
        u16  first_cluster_high;    /* High 16 bits of this entry's first cluster */
        u16  last_modified_time;    /* Yep ignored too */
        u16  last_modified_date;    /* Same case */
        u16  first_cluster_low;     /* Low 16 bits of this entry's first cluster */
        u32  size;                  /* Size of the file in bytes */
} DirectoryEntry;

/* Internal driver data */
typedef struct _FAT32FileData {
        u32       first_cluster; /* First cluster of this file */
        Byte      attributes;    /* Attributes */
        Partition part;          /* Partition this file belongs to */
} FAT32FileData;

_Static_assert(sizeof(BIOSParameterBlock) == SECTOR_SIZE,
               "BIOS Parameter Block is not aligned correctly!");
_Static_assert(sizeof(FSInfo) == SECTOR_SIZE, "FSInfo structure is not aligned correctly!");

/* Returns the LBA of the first FAT */
static inline u32 FindFAT(const Partition *p, BIOSParameterBlock *bpb) {
        return (p->lba_start + bpb->reserved_sectors);
}

static inline u32 FindDataStart(const Partition *p, BIOSParameterBlock *bpb) {
        return FindFAT(p, bpb) + bpb->n_fats * bpb->sectors_per_fat;
}

static inline u32 ClusterToLBA(const Partition *p, BIOSParameterBlock *bpb, u32 c) {
        /* First two clusters are reserved */
        return FindDataStart(p, bpb) + (c - 2) * bpb->sectors_per_cluster;
}

static u32 ReadFAT32Entry(const Partition *p, BIOSParameterBlock *bpb, u32 cluster) {
        u32 fat_offset = cluster * sizeof(u32);
        u32 fat_sector = FindFAT(p, bpb) + (fat_offset / SECTOR_SIZE);
        u32 ent_offset = fat_offset % SECTOR_SIZE;

        Byte buf[512];
        ATAReadSectors(0, 1, fat_sector, buf);

        u32 entry = *(u32 *)(buf + ent_offset);
        return entry & 0x0FFFFFFF; /* The highest 4 bits are reserved in FAT32, so each entry is
                                      only 28 bits wide */
}

/*
 * Simple utility function to convert 8.3 filenames to normal filenames
 * eg: KERNEL  SYS -> KERNEL.SYS
 * This also converts the name to uppercase, as FAT expects uppercase names.
 */
static void FATEntryToString(const char *raw, char *dest) {
        int d_ptr = 0;

        for (int i = 0; i < 8 && raw[i] != ' '; i++) {
                /* FAT only knows uppercase names, so we must also convert the string to uppercase
                 * while we're at it. */
                dest[d_ptr++] = (char)toupper((unsigned char)raw[i]);
        }

        if (raw[8] != ' ') {
                dest[d_ptr++] = '.';
                for (int i = 8; i < 11 && raw[i] != ' '; i++) {
                        dest[d_ptr++] = (char)toupper((unsigned char)raw[i]);
                }
        }

        dest[d_ptr] = '\0';
}

static Status ScanDirectoryFor(const Partition p, BIOSParameterBlock *bpb, u32 cluster,
                               const char *filename, File *output) {
        Byte sector[512];
        /* Clusters 0 and 1 are reserved in FAT */
        while (cluster < FAT32_EOF && cluster >= 2) {
                u32 lba = ClusterToLBA(&p, bpb, cluster);

                for (int s = 0; s < bpb->sectors_per_cluster; s++) {
                        ATAReadSectors(0, 1, lba + s, sector);

                        for (int i = 0; i < SECTOR_SIZE; i += sizeof(DirectoryEntry)) {
                                Byte *ent = sector + i;

                                DirectoryEntry *de = (DirectoryEntry *)ent;
                                if (de->name[0] == 0x00) return STATUS_NOT_FOUND;
                                if (de->name[0] == (char)0xE5) continue; /* Deleted file, skip */
                                if (de->attributes == 0x0F) continue; /* LFN entry, skip for now */

                                char filename83[13];
                                memset(filename83, 0, sizeof(filename83));
                                memcpy(filename83, de->name, 8);
                                memcpy(filename83 + 8, de->extension, 3);
                                char entry_name[13];
                                FATEntryToString(filename83, entry_name);

                                if (strcmp(entry_name, filename) == 0) {
                                        /* Combine the low and high bits into one u32, ensure the
                                         * entry is only 28 bits by masking the top four ones */
                                        u32 first_cluster = (((u32)de->first_cluster_high << 16) |
                                                             (u32)de->first_cluster_low) &
                                                            0x0FFFFFFF;
                                        FAT32FileData *private = kmalloc(sizeof(FAT32FileData));
                                        if (!private) return STATUS_OUT_OF_MEMORY;

                                        private->first_cluster = first_cluster;
                                        private->attributes    = de->attributes;
                                        private->part          = p;

                                        output->cursor   = 0;
                                        output->ended    = false;
                                        output->internal = private;
                                        output->loaded   = true;
                                        output->filesize = de->size;
                                        return STATUS_OK;
                                }
                        }
                }

                cluster = ReadFAT32Entry(&p, bpb, cluster);
                if (cluster == FAT32_BAD_CLUSTER) return STATUS_NOT_FOUND;
        }
        return STATUS_NOT_FOUND;
}

Status OpenFile_FAT32(const Partition p, const char *path, File *output) {
        output->t = PART_FAT32_LBA;
        BIOSParameterBlock bpb;
        ATAReadSectors(0, 1, (u32)p.lba_start, (Byte *)&bpb);

        return ScanDirectoryFor(p, &bpb, bpb.root_cluster, path, output);
}

int ReadFile_FAT32(File *f, Size n_bytes, void *output) {
        FAT32FileData     *data = f->internal;
        BIOSParameterBlock bpb;
        if (ATAReadSectors(0, 1, (u32)data->part.lba_start, (Byte *)&bpb) < 1) return -1;
        u32 bytes_per_cluster = bpb.sectors_per_cluster * SECTOR_SIZE;
        u32 cluster           = data->first_cluster;

        u32 clusters_to_skip = f->cursor / bytes_per_cluster;
        for (u32 i = 0; i < clusters_to_skip; i++) {
                cluster = ReadFAT32Entry(&data->part, &bpb, cluster);
                if (cluster >= FAT32_EOF) return 0;
        }

        u32 bytes_read = 0;
        while (bytes_read < n_bytes && cluster < FAT32_EOF) {
                u32 lba = ClusterToLBA(&data->part, &bpb, cluster);

                u32 offset_in_cluster = f->cursor % bytes_per_cluster;
                u32 sector_in_cluster = offset_in_cluster / SECTOR_SIZE;
                u32 offset_in_sector  = offset_in_cluster % SECTOR_SIZE;

                for (u32 s = sector_in_cluster; s < bpb.sectors_per_cluster && bytes_read < n_bytes;
                     s++) {
                        Byte sector_temp[SECTOR_SIZE];
                        if (ATAReadSectors(0, 1, lba + s, sector_temp) < 1) return -1;

                        u32 can_read = SECTOR_SIZE - offset_in_sector;
                        u32 to_copy  = (n_bytes - bytes_read < can_read) ? (n_bytes - bytes_read)
                                                                         : can_read;

                        memcpy((Byte *)output + bytes_read, sector_temp + offset_in_sector,
                               to_copy);

                        bytes_read += to_copy;
                        f->cursor += to_copy;
                        offset_in_sector = 0;
                }

                cluster = ReadFAT32Entry(&data->part, &bpb, cluster);
        }

        return bytes_read;
}
