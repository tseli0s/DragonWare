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
        int whoami;
} thread_data[2];
static int n_thread_data = 0;

static inline void die(const char *msg) {
        printf("idedrv: fatal error: %s\n", msg);
        exit(EXIT_FAILURE);
}

static void listener(void *data) {
        struct __data listener_data;
        memcpy(&listener_data, data, sizeof(struct __data));
        int         whoami    = listener_data.whoami;
        const char *busstr    = (whoami == 0) ? "primary" : "secondary";
        char        fmtbuf[6] = {0};
        snprintf(fmtbuf, sizeof(fmtbuf), "HD%d", whoami);

        Handle h   = CreateObject(fmtbuf, OBJ_PORT, 0);
        Handle irq = CreateObject(NullPointer, OBJ_PORT, 0);

        if (h < 0) {
                printf("warning: Cannot create listener port for %s bus, disabling access", busstr);
                return;
        }

        if (irq < 0) {
                /* TODO: Have a pure polling fallback here maybe */
                printf("warning: cannot create IRQ dispatch port for %s bus, "
                       "disabling access to bus",
                       busstr);
                return;
        }

        if (InvokeObject(h, PORT_CREATE, NullPointer) != STATUS_OK) {
                printf("warning: Cannot expose communication port"
                       "for %s bus, disabling access",
                       busstr);
                goto fail;
        }
        if (InvokeObject(irq, PORT_CREATE, NullPointer) != STATUS_OK) {
                printf("warning: Cannot create IRQ dispatch port for %s bus", busstr);
                goto fail;
        }

        IRQBindingDescriptor irq_descr = {.irq_no = (whoami == 0) ? 14 : 15, .reserved = 0};
        if (InvokeObject(irq, PORT_BIND_IRQ, &irq_descr) != STATUS_OK) {
                printf("error: Cannot bind to IRQ %d", irq_descr.irq_no);
                goto fail;
        }

        Message m;
        while (true) {
                if (ReceiveMessage(h, &m) != STATUS_OK) continue;

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
fail:
        if (h >= 0) DeleteObject(h);
        if (irq >= 0) DeleteObject(irq);

        return;
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

        Handle t1 = CreateObject(NullPointer, OBJ_THREAD, 0);
        Handle t2 = CreateObject(NullPointer, OBJ_THREAD, 0);
        if (t1 < 0 || t2 < 0) die("Cannot allocate objects for IRQ14/15 rerouting");

        /* yes. i REALLY need to implement malloc here. */
        thread_data[0] = (struct __data){.whoami = 0};
        thread_data[1] = (struct __data){.whoami = 1};
        for (int i = 0; i <= 1; i++) {
                Handle                t       = (!i) ? t1 : t2;
                Handle                section = CreateObject(NullPointer, OBJ_SECTION, 0);
                UserSectionDescriptor sectreq = {.needed_pages = 4,
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

        /* The other listener threads do the rest of the job, technically we should block entirely
         * here, but I don't have a function for this yet. */
        while (true) {
                _DWYield();
        }
        return 0;
}
