/**********************************************************************
 * FILE: main.c
 * PURPOSE: Volume manager for DragonWare userland
 * PROJECT: DragonWare Base System
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <macros.h>
#include <message.h>
#include <object.h>
#include <object/port_object.h>
#include <object/section_object.h>
#include <stdio.h>
#include <string.h>

#include "idedrv/protocol.h"
#include "mbr.h"

#define MAX_PROBE_RETRIES (5)

typedef struct _DiskDevice {
        char   id[12];
        Handle port;
} DiskDevice;

static DiskDevice devs[128] = {0};
static int        ndevs     = 0;

int main(void) {
        int per_process_objects = _DWSystemQuery(SQ_N_OBJECTS_PER_PROCESS, NullPointer);
        for (int i = 0; i < per_process_objects; i++) {
                char fmtbuf[12];
                snprintf(fmtbuf, sizeof(fmtbuf), "HD%d", i);
                Handle hd = OpenPort(fmtbuf);

        /*
         * FIXME: Here's what happens: There's actually a race condition here. The drivers may need
         * time to initialize the ports, but may be preempted by the kernel in the meantime and the
         * kernel may schedule this process next. But even though the disk drivers are ready to
         * create the ports, this driver can't find them yet, so we loop and hope the disk drivers
         * are rescheduled in the meantime. So it all depends on kernel scheduler behaviour.
         *
         * It's a very shitty workaround, so if you're reading this and you have a better idea, I'd
         * seriously beg you to tell me. You can't imagine how much I hate reading this piece of
         * code.
         */
        tryloop:
                if (hd < 0) {
                        if (i == 0) {
                                for (int retry = 0; retry < MAX_PROBE_RETRIES; retry++) {
                                        hd = OpenPort(fmtbuf);
                                        goto tryloop;
                                }
                        } else
                                continue;
                }

                devs[ndevs] = (DiskDevice){
                        .port = hd,
                };
                strncpy(devs[ndevs].id, fmtbuf, sizeof(fmtbuf));
                ndevs++;
        }
        if (!ndevs) {
                puts("volmgr: error: No devices found");
                return -1;
        }

        for (int i = 0; i < ndevs; i++) {
                DiskDevice current = devs[i];
                Handle     shmem   = RequestMemorySection(
                        1, SECTION_SHAREABLE | SECTION_WRITEABLE | SECTION_CACHEABLE);
                void *base;
                if (shmem < 0) {
                        printf("volmgr: During iteration %d/%d, RequestMemorySection() failed. "
                               "Exiting.",
                               i, ndevs);
                        return -1;
                }

                if (MapMemorySection(shmem, &base) != STATUS_OK) {
                        puts("volmgr: Failed to map memory section");
                        return -1;
                }

                Handle tmprecv = CreatePort(NullPointer);
                if (tmprecv < 0) return -1;

                Message m               = {0};
                m.header.protocol       = IDEDRV_PROTOCOL_V0;
                m.header.payload_length = sizeof(IDEDRVRequest);
                m.header.type           = IDEDRV_READ_SECTOR;
                IDEDRVRequest req       = {.lba            = 0,
                                           .shared_section = shmem,
                                           /* FIXME: Handle slave devices on a bus in idedrv */
                                           .master         = 0,
                                           .__reserved     = 0};
                memcpy(m.payload.raw, &req, sizeof(IDEDRVRequest));

                if (IPCCall(&m, current.port, tmprecv) != STATUS_OK) return -1;
                DeleteObject(tmprecv);

                MBRTable table;
                Status   status = ReadMBRTable(base, table);
                DeleteObject(shmem);

                if (status != STATUS_OK) {
                        /* There's probably no MBR table here, no point in trying the dump below */
                        continue;
                } else {
                        for (int j = 0; j < MAX_MBR_ENTRIES; j++) {
                                MBREntry this  = table[j];
                                uint32_t start = this.lba_first_absolute_sector;
                                uint32_t end =
                                        this.lba_first_absolute_sector + this.n_sectors_partition;
                                if (!this.partition_type || !start) continue;

                                printf("idedrv: Device %d:%d: LBAs 0x%x-0x%x, "
                                       "active: %d, type: 0x%x\n",
                                       i, j, start, end, (this.status == 0x80) ? 1 : 0,
                                       this.partition_type);
                                /* And this is where I'd create the volume listener ports, let me
                                 * think of how to name them... */
                        }
                }
        }
        return 0;
}
