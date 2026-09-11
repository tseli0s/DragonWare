/**********************************************************************
 * FILE: rw.c
 * PURPOSE: Read/Write I/O operations for ATA/IDE drives
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHORS: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "rw.h"

#include <io.h>
#include <ipc86.h>
#include <kernelapi.h>
#include <kerneltypes.h>
#include <macros.h>
#include <message.h>
#include <object.h>
#include <object/port_object.h>
#include <spinlock.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ctrl.h"
#include "idedrv/protocol.h"
#include "portdef.h"

typedef struct _CachedSector {
        int  valid;
        int  bus, master;
        LBA  lba;
        Byte data[ATA_SECTOR_SIZE];
} CachedSector;

static Spinlock     cache_spinlock;
static CachedSector cache[16]      = {0};
static int          n_cache_stored = 0;

/* Um maybe have a better name here idk */
#define CACHE_ACCESSING_CODE(__spinlock, __LAMBDA__) \
        do {                                         \
                AcquireSpinlock(&(__spinlock));      \
                __LAMBDA__                           \
                ReleaseSpinlock(&(__spinlock));      \
        } while (0)

[[gnu::hot]]
static void LoadCache(int bus, int master, LBA lba, Byte *data) {
        CACHE_ACCESSING_CODE(cache_spinlock, {
                cache[n_cache_stored].valid  = 1;
                cache[n_cache_stored].bus    = bus;
                cache[n_cache_stored].master = master;
                cache[n_cache_stored].lba    = lba;
                memcpy(cache[n_cache_stored].data, data, ATA_SECTOR_SIZE);

                n_cache_stored++;
                if ((unsigned)n_cache_stored >= arraysize(cache)) n_cache_stored = 0;
        });
}

[[gnu::nonnull]]
static Bool CheckCacheForSector(int bus, int master, LBA sector, int *pos) {
        *pos       = -1;
        Bool found = false;
        CACHE_ACCESSING_CODE(cache_spinlock, {
                for (int i = 0; i < (int)arraysize(cache); i++)
                        if (cache[i].lba == sector && cache[i].bus == bus &&
                            cache[i].master == master && cache[i].valid != 0) {
                                *pos  = i;
                                found = true;
                                break;
                        }
        });
        return found;
}

static void ReadFromCache(int index, void *buf) {
        if (index < 0) return;
        CACHE_ACCESSING_CODE(cache_spinlock, {
                if (cache[index].valid) memcpy(buf, cache[index].data, ATA_SECTOR_SIZE);
        });
}

static inline void InvalidateCache(int bus, int master, LBA sector) {
        CACHE_ACCESSING_CODE(cache_spinlock, {
                for (Size i = 0; i < arraysize(cache); i++)
                        if (cache[i].lba == sector && cache[i].bus == bus &&
                            cache[i].master == master) {
                                memset(&cache[i], 0, sizeof(CachedSector));
                        }
        });
}

static inline Bool CheckForError(int bus) {
        u16  port   = (bus == 0) ? ATA_STATUS_PRIMARY : ATA_STATUS_SECONDARY;
        Byte status = inb(port);
        return status & (ATA_STATUS_ERROR | ATA_STATUS_DF);
}

static inline void WaitBSYClear(int bus) {
        u16 port = (bus == 0) ? ATA_ALTSTATUS_PRIMARY : ATA_ALTSTATUS_SECONDARY;
        while (inb(port) & ATA_STATUS_BSY);
}

static inline void WaitForDRQ(int bus) {
        u16 port = (bus == 0) ? ATA_STATUS_PRIMARY : ATA_STATUS_SECONDARY;
        while (!(inb(port) & ATA_STATUS_DRQ));
}

static inline void SelectDevice(u32 lba, int bus, int master) {
        u16 port = (bus == 0) ? ATA_HDDEVSEL_PRIMARY : ATA_HDDEVSEL_SECONDARY;
        outb(port, (Byte)((u32)0xE0 | ((unsigned)master << 4ULL) | ((lba >> 24) & 0x0F)));

        Wait400ns(bus);
}

static inline void SendSectorCount(int bus) {
        u16 port = (bus == 0) ? ATA_SECCOUNT_PRIMARY : ATA_SECCOUNT_SECONDARY;
        outb(port, 1);
}

static inline void SubmitLBA(int bus, u32 lba) {
        u16 port0 = (bus == 0) ? ATA_LBA0_PRIMARY : ATA_LBA0_SECONDARY;
        u16 port1 = (bus == 0) ? ATA_LBA1_PRIMARY : ATA_LBA1_SECONDARY;
        u16 port2 = (bus == 0) ? ATA_LBA2_PRIMARY : ATA_LBA2_SECONDARY;

        outb(port0, (Byte)(lba >> 0));
        outb(port1, (Byte)(lba >> 8));
        outb(port2, (Byte)(lba >> 16));
}

static inline void RequestRead(int bus) {
        u16 port = (bus == 0) ? ATA_COMMAND_PRIMARY : ATA_COMMAND_SECONDARY;
        outb(port, ATA_CMD_READ);
}

static inline void RequestWrite(int bus) {
        u16 port = (bus == 0) ? ATA_COMMAND_PRIMARY : ATA_COMMAND_SECONDARY;
        outb(port, ATA_CMD_WRITE);
}

static inline void FlushWriteCache(int bus) {
        u16 port = (bus == 0) ? ATA_COMMAND_PRIMARY : ATA_COMMAND_SECONDARY;
        outb(port, ATA_CMD_CACHE_FLUSH);
}

/* Apparently, reading and writing to an ATA drive is very similar code wise... */
static void PrepareDriveForRW(int bus, int master, u32 lba) {
        lba &= 0x0FFFFFFF;

        /*
         * look at this cool trick, bounding bus and master between 0 and 1 using this boolean trick
         * just leaving a comment here, because it may look weird on first glance
         */
        bus    = !!(bus);
        master = !!(master);

        SelectDevice(lba, bus, master);
        WaitBSYClear(bus);

        SendSectorCount(bus);
        SubmitLBA(bus, lba);
}

IDEDRVStatusReply ReadFromDisk(Handle irq_handle, int bus, int master, u32 lba, void *buf) {
        int pos;
        CheckCacheForSector(bus, master, lba, &pos);
        if (pos >= 0) {
                ReadFromCache(pos, buf);
                return IDEDRV_SUCCESS;
        }

        PrepareDriveForRW(bus, master, lba);
        RequestRead(bus);

        Bool    anything_our_way = false;
        Message m;
        while (!anything_our_way) {
                if (ReceiveMessage(irq_handle, &m) != STATUS_OK) continue;

                if (m.header.sender != KERNEL_SENDER) {
                        puts("idedrv: received message on IRQ bound port but sender is not "
                             "KERNEL_SENDER, ignoring");
                        _DWYield();
                        continue;
                }

                anything_our_way = true;
                AcknowledgeIRQ(irq_handle, (bus == 0) ? 14 : 15);

                if (CheckForError(bus)) {
                        printf("idedrv: drive error after READ command\n");
                        return IDEDRV_BUG_CHECK;
                }
                u16  port = (bus == 0) ? ATA_DATA_PRIMARY : ATA_DATA_SECONDARY;
                u16 *out  = buf;
                for (int i = 0; i < 256; i++) out[i] = inw(port);
        }

        LoadCache(bus, master, lba, buf);
        Wait400ns(bus); /* let the drive flush down any stale data and prepare it for the next
                           command */
        return IDEDRV_SUCCESS;
}

/*
 * FIXME: This is a pure polling driver. I tried doing it using interrupts like ReadFromDisk()
 * above, but it blocked and no interrupts came. So for now, polling and wasting CPU cycles it is,
 * until I grab a copy of the specification and read what happens.
 */
IDEDRVStatusReply WriteToDisk(Handle irq_handle, int bus, int master, u32 lba, void *buf) {
        UNUSED(irq_handle);

        DisableINTRQ(bus);
        WaitBSYClear(bus);
        PrepareDriveForRW(bus, master, lba);
        RequestWrite(bus);
        WaitForDRQ(bus);

        u16  port = (bus == 0) ? ATA_DATA_PRIMARY : ATA_DATA_SECONDARY;
        u16 *src  = buf;
        for (int i = 0; i < 256; i++) {
                outw(port, src[i]);
                /* "There must be a tiny delay between each OUTSW output uint16_t. A jmp $+2 size of
                 * delay.". -- https://wiki.osdev.org/ATA_PIO_Mode
                 *
                 * Um, I hope this is good enough :P
                 */
                __asm__ volatile(
                        "jmp 1f\n"
                        "1: nop\n");
        }
        Wait400ns(bus); /* let the drive flush down any stale data and prepare it for the
                           next command */
        FlushWriteCache(bus);
        WaitBSYClear(bus);

        EnableINTRQ(bus);

        InvalidateCache(bus, master, lba);
        return IDEDRV_SUCCESS;
}
