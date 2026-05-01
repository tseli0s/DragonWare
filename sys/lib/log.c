/**********************************************************************
 * FILE: log.c
 * PURPOSE: Kernel logging support
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "log.h"

#include <kstring.h>
#include <ktypes.h>
#include <macros.h>
#include <mmutils.h>

#include "iomgr/class.h"
#include "iomgr/node.h"
#include "video/output.h"
#include "video/pixels.h"

#define BUFSIZE 32
#ifndef LOG_BUFSIZE
#define LOG_BUFSIZE (64)
#endif

[[gnu::aligned(4)]]
static LogMessage logbuffer[LOG_BUFSIZE] = {0};
static Size       logbuf_idx             = 0;
static Bool       logs_flushed           = false;

static void PrintToOutputs(const char *msg, PixelColor bg, PixelColor fg) {
        Size len = strlen(msg);
#ifdef DRAGONWARE_DEBUG_MODE
        ForEachConsoleDevice({
                DeviceManagerNode *out = curr->node; /* curr given by the macro */
                out->devtable.ddo->console.SetTextAttributes(out->private_state, bg, fg);
                for (Size i = 0; i < len; i++) {
                        out->devtable.ddo->console.WriteSingleChar(out->private_state, msg[i]);
                }
                out->devtable.ddo->console.SetTextAttributes(out->private_state, BlackPixel,
                                                             WhitePixel);
        });
#else
        OutputNode *out = GetPrimaryOutputDevice();
        if (!out) return; /* No output to print on, wait for devices to come online*/
        DeviceManagerNode *dev = out->node;
        dev->devtable.ddo->console.SetTextAttributes(dev->private_state, bg, fg);
        for (Size i = 0; i < len; i++)
                out->node->devtable.ddo->console.WriteSingleChar(out->node->private_state, msg[i]);
#endif /* DRAGONWARE_DEBUG_MODE */
}

static void PrintPrefixFor(LogLevel level) {
        char      *prefix = "????? ";
        PixelColor fg     = WhitePixel;
        switch (level) {
                case LOG_DEBUG: {
                        prefix = "* DEBUG   ";
                        fg     = CyanPixel;
                        break;
                }
                case LOG_INFO: {
                        prefix = "* INFO    ";
                        fg     = GreenPixel;
                        break;
                }
                case LOG_WARNING: {
                        prefix = "* WARNING ";
                        fg     = OrangePixel;
                        break;
                }
                case LOG_ERROR: {
                        prefix = "* ERROR   ";
                        fg     = RedPixel;
                        break;
                }
        }
        PrintToOutputs(prefix, BlackPixel, fg);
}

void LogInit(void) {
        ZeroMemory(logbuffer);
        logbuf_idx = 0;
}

[[gnu::hot]]
void klog(LogLevel level, const char *fmt, ...) {
        char    buf[LOG_MAXBUF];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        /* Avoid printing duplicate logs or inserting new logs in the middle of early ones by simply
         * checking if the logs were flushed before. If so, we can now start printing. */
        if (logs_flushed) {
                PrintPrefixFor(level);
                PrintToOutputs(buf, BlackPixel, WhitePixel);
                PrintToOutputs("\n", BlackPixel, WhitePixel);
        }

        if ((logbuf_idx + 1) >= arraysize(logbuffer)) return;
        logbuffer[logbuf_idx] = (LogMessage){.buffer = {0}, /* Needs to be copied manually */
                                             .code   = level};
        strncpy(logbuffer[logbuf_idx].buffer, buf, LOG_MAXBUF);
        logbuffer[logbuf_idx].buffer[LOG_MAXBUF - 1] = '\0';
        logbuf_idx++;
}

void FlushAllLogs(void) {
        if (logs_flushed) return;

        for (Size i = 0; i < logbuf_idx; i++) {
                PrintPrefixFor(logbuffer[i].code);
                PrintToOutputs(logbuffer[i].buffer, BlackPixel, WhitePixel);
                PrintToOutputs("\n", BlackPixel, WhitePixel);
        }
        logs_flushed = true;
}
