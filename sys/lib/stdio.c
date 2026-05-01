/**********************************************************************
 * FILE: stdio.c
 * PURPOSE: Standard I/O libc functions for the kernel
 * PROJECT: DragonWare Freestanding Library
 * DATE: 23-02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kmalloc.h>
#include <kstring.h>
#include <mmutils.h>

#include "iomgr/class.h"
#include "iomgr/node.h"
#include "video/output.h"

int printf(const char *fmt, ...) {
        char writebuf[256] = {0};

        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(writebuf, sizeof(writebuf), fmt, ap);
        va_end(ap);

#ifdef DRAGONWARE_DEBUG_MODE
        ForEachConsoleDevice({
                for (Size i = 0; i < sizeof(writebuf); i++) {
                        if (writebuf[i] == 0) break;
                        DeviceManagerNode *out = curr->node;
                        out->devtable.ddo->console.WriteSingleChar(out->private_state, writebuf[i]);
                }
        });
        goto cleanup;
#else
        for (Size i = 0; i < sizeof(writebuf); i++) {
                if (writebuf[i] == 0) break;
                OutputNode *out = GetPrimaryOutputDevice();
                if (!out) goto cleanup;
                out->node->devtable.ddo->console.WriteSingleChar(out->node->private_state,
                                                                 writebuf[i]);
        }
        goto cleanup;
#endif /* DRAGONWARE_DEBUG_MODE */
cleanup:
        return n;
}

char *kstrdup(const char *src) {
        const Size len = strlen(src);
        char      *ptr = kmalloc(len);
        memcpy(ptr, src, len);
        ptr[len] = '\0';

        return ptr;
}
