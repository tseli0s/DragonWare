/**********************************************************************
 * FILE: idedrv.c
 * PURPOSE: IDE/ATA userspace disk driver entry point
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <message.h>
#include <object.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "idedrv/protocol.h"
#include "identify.h"
#include "portdef.h"

/* I should really implement malloc guys */
static struct __data {
        int    whoami;
        Handle h;
} thread_data[2];
static int n_thread_data = 0;

static inline void die(const char *msg) {
        printf("idedrv: fatal error: %s\n", msg);
        exit(EXIT_FAILURE);
}

static void listener(void *data) {
        struct __data listener_data;
        memcpy(&listener_data, data, sizeof(struct __data));

        Message m;
        while (true) {
                if (ReceiveMessage(listener_data.h, &m) != STATUS_OK) continue;

                if (m.header.protocol != IDEDRV_PROTOCOL_V0) continue;
                switch (m.header.type) {
                        /* TODO: Implement replying (and in this case say I'm working on this). */
                        case IDEDRV_READ_SECTOR:
                        case IDEDRV_WRITE_SECTOR:
                        default:
                                _DWYield();
                                continue;
                }
        }
}

int main(void) {
        const u16 ata_ports[] = {
                ATA_CTRL_PRIMARY,       ATA_CTRL_SECONDARY,     ATA_PRIMARY_BASE,
                ATA_SECONDARY_BASE,     ATA_PRIMARY_BASE + 1,   ATA_PRIMARY_BASE + 2,
                ATA_PRIMARY_BASE + 3,   ATA_PRIMARY_BASE + 4,   ATA_PRIMARY_BASE + 5,
                ATA_PRIMARY_BASE + 6,   ATA_PRIMARY_BASE + 7,   ATA_SECONDARY_BASE + 1,
                ATA_SECONDARY_BASE + 2, ATA_SECONDARY_BASE + 3, ATA_SECONDARY_BASE + 4,
                ATA_SECONDARY_BASE + 5, ATA_SECONDARY_BASE + 6, ATA_SECONDARY_BASE + 7,
        };
        if (_DWRequestPorts(ata_ports, sizeof(ata_ports) / sizeof(ata_ports[0])) != STATUS_OK)
                die("Cannot access ATA/IDE I/O ports, permission denied from the kernel.");

        IRQBindingDescriptor irq14 = {
                .irq_no   = 14,
                .reserved = 0,
        };
        IRQBindingDescriptor irq15 = {
                .irq_no   = 14,
                .reserved = 0,
        };
        Handle h1 = CreateObject(NullPointer, OBJ_PORT, 0);
        Handle h2 = CreateObject(NullPointer, OBJ_PORT, 0);
        Handle t1 = CreateObject(NullPointer, OBJ_THREAD, 0);
        Handle t2 = CreateObject(NullPointer, OBJ_THREAD, 0);
        if (h1 < 0 || h2 < 0 || t1 < 0 || t2 < 0)
                die("Cannot allocate objects for IRQ14/15 rerouting");

        if (InvokeObject(h1, PORT_CREATE, NullPointer) != STATUS_OK)
                die("Cannot create port for IRQ14 binding");
        if (InvokeObject(h2, PORT_CREATE, NullPointer) != STATUS_OK)
                die("Cannot create port for IRQ15 binding");

        if (InvokeObject(h1, PORT_BIND_IRQ, &irq14) != STATUS_OK) die("Cannot bind to IRQ14");
        if (InvokeObject(h2, PORT_BIND_IRQ, &irq15) != STATUS_OK) die("Cannot bind to IRQ15");

        /* yes. i REALLY need to implement malloc here. */
        thread_data[0] = (struct __data){.h = h1, .whoami = 0};
        thread_data[1] = (struct __data){.h = h2, .whoami = 1};
        for (int i = 0; i <= 1; i++) {
                Handle                t       = (!i) ? t1 : t2;
                Handle                section = CreateObject(NullPointer, OBJ_SECTION, 0);
                UserSectionDescriptor sectreq = {.needed_pages = 2,
                                                 .perms = SECTION_CACHEABLE | SECTION_WRITEABLE};
                uintptr_t             stackaddr;
                if (section < 0) die("Cannot allocate thread stack");
                if (InvokeObject(section, SECTION_REQUEST, &sectreq) != STATUS_OK)
                        die("Kernel refused section request");
                if (InvokeObject(section, SECTION_MAP, &stackaddr) != STATUS_OK)
                        die("Cannot map new thread stack");

                UserThreadData descr = {
                        .entry = listener,
                        .stack = (void *)(stackaddr + (2 * 0x1000)), /* FIXME: Magic number here */
                        .extra_data = &thread_data[n_thread_data]};
                if (InvokeObject(t, THREAD_CREATE, &descr) != STATUS_OK)
                        die("can't create new thread");
                if (InvokeObject(t, THREAD_RUN, NullPointer) != STATUS_OK)
                        die("can't run new thread");
                n_thread_data++;
        }

        int n = 0;
        for (int bus = 0; bus <= 1; bus++) {
                for (int master = 0; master <= 1; master++) {
                        if (IdentifyDrive(bus, master) != STATUS_OK)
                                continue;
                        else {
                                printf("Detected connected ATA/IDE compatible medium on bus %d "
                                       "drive %d\n",
                                       bus, master);
                                n++;
                        }
                }
        }
        if (!n) die("No connected ATA/IDE drives, unloading idedrv driver");
        while (true) {
                _DWYield();
        }
        return 0;
}
