/**********************************************************************
 * FILE: initdriver.c
 * PURPOSE: Driver loading/initialization routines
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ktypes.h>
#include <log.h>
#include <mmutils.h>

#include "ddk/ia32/idt.h"
#include "driversdk.h"

extern DriverDescriptor *__driver_descriptor_start[];
extern DriverDescriptor *__driver_descriptor_end[];

#define ForEachDescriptor(__LAMBDA__)                                      \
        do {                                                               \
                for (DriverDescriptor **_iter = __driver_descriptor_start; \
                     _iter < __driver_descriptor_end; _iter++) {           \
                        __LAMBDA__                                         \
                }                                                          \
                                                                           \
        } while (0)

/* We'll do the initialization in two passes; First the drivers that set init_earlier to true, then
 * those than didn't. It's a quick way of initializing drivers needed by the kernel before we get
 * fancy stuff working, or help with debugging without a screen */
Status BringBuiltinDriversOnline(void) {
        Size i = 0;

        ForEachDescriptor({
                DriverDescriptor *d = *_iter;
                if (!d || !d->__init || !d->init_earlier) continue;

                Status driver_result = d->__init();
                if (driver_result != STATUS_OK) {
                        LogMessage(LOG_WARNING, "Early driver '%s' failed (%s), skipping", d->name,
                                   StatusCodeToString(driver_result));
                        continue;
                } else
                        i++;
        });

        ForEachDescriptor({
                DriverDescriptor *d = *_iter;
                if (!d || !d->__init || d->init_earlier) continue;

                Status driver_result = d->__init();
                if (driver_result != STATUS_OK) {
                        LogMessage(LOG_WARNING, "Driver '%s' failed (%s), skipping", d->name,
                                   StatusCodeToString(driver_result));
                        continue;
                } else
                        i++;
        });

        if (i > 0) LogMessage(LOG_INFO, "%d builtin drivers initialized", i);

        return STATUS_OK;
}
