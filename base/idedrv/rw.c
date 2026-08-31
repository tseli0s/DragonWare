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
#include <message.h>
#include <object.h>
#include <stdio.h>

#include "ctrl.h"
#include "idedrv/protocol.h"
#include "portdef.h"

/* TODO: Synchronization and lock-free mechanisms here (the whole file), because apparently some
 * old drives can lock up if we do multiple things without synchronization and submit unrelated data
 * each time. Not a problem for the short term future, but we should address it. */

static inline Bool CheckForError(int bus) {
        u16  port   = (bus == 0) ? ATA_STATUS_PRIMARY : ATA_STATUS_SECONDARY;
        Byte status = inb(port);
        return status & (ATA_STATUS_ERROR | ATA_STATUS_DF);
}

static inline void WaitBSYClear(int bus) {
        u16 port = (bus == 0) ? ATA_ALTSTATUS_PRIMARY : ATA_ALTSTATUS_SECONDARY;
        while (inb(port) & ATA_STATUS_BSY);
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

IDEDRVStatusReply ReadFromDisk(Handle irq_handle, IRQBindingDescriptor irq_descr, int bus,
                               int master, u32 lba, void *buf) {
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

        RequestRead(bus);

        Bool    anything_our_way = false;
        Message m;
        while (!anything_our_way) {
                if (ReceiveMessage(irq_handle, &m) != STATUS_OK) continue;
        
                if (m.header.sender != KERNEL_SENDER) {
                        puts("idedrv: received message on IRQ bound port but sender is not KERNEL_SENDER, ignoring");
                        _DWYield();
                        continue;
                }
        
                anything_our_way = true;
                InvokeObject(irq_handle, PORT_ACK_IRQ, &irq_descr);
        
                if (CheckForError(bus)) {
                        printf("idedrv: drive error after READ command\n");
                        return IDEDRV_BUG_CHECK;
                }
        
                u16  port = (bus == 0) ? ATA_DATA_PRIMARY : ATA_DATA_SECONDARY;
                u16 *out  = buf;
                for (int i = 0; i < 256; i++) {
                        out[i] = inw(port);
                }
        }

        Wait400ns(bus); /* let the drive flush down any stale data and prepare it for the next
                           command */
        return IDEDRV_SUCCESS;
}
