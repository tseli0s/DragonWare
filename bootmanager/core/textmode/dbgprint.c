/**********************************************************************
 * FILE: dbgprint.c
 * PURPOSE: COM1 serial output implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <ioport.h>
#include <kstring.h>
#include <stdarg.h>

#ifndef BOOTMGR_BRANDING_STRING /* I might make this configurable in the future in \
                                   CMakeLists.txt... */
#define BOOTMGR_BRANDING_STRING " * BOOT   "
#endif /* BOOTMGR_BRANDING_STRING */

#ifndef DRAGONWARE_DEBUG_MODE
#include "macros.h" /* UnusedParameter macro */
#endif              /* DRAGONWARE_DEBUG_MODE */

#ifdef DRAGONWARE_DEBUG_MODE
typedef enum _SerialPort { COM1_PORT = 0x3F8, COM2_PORT = 0x2F8 } SerialPort;
static inline void WaitForPort1(void) { while (!(inb(COM1_PORT + 5) & 0x20)); }
static inline void WriteToSerialPort1(char c) {
        WaitForPort1();
        outb(COM1_PORT, (Byte)c);
}

static void WriteSerialChar(char c) {
        if (c == '\n') {
                WriteToSerialPort1('\r');
                WriteToSerialPort1('\n');
        } else
                WriteToSerialPort1(c);
}
#endif /* DRAGONWARE_DEBUG_MODE */

void InitDebugPrint(void) {
#ifdef DRAGONWARE_DEBUG_MODE
        outb(COM1_PORT + 1, 0x00);
        outb(COM1_PORT + 3, 0x80);
        outb(COM1_PORT, 0x03);
        outb(COM1_PORT + 1, 0x00);
        outb(COM1_PORT + 3, 0x03);
        outb(COM1_PORT + 2, 0xC7);
        outb(COM1_PORT + 4, 0x0B);
#endif /* DRAGONWARE_DEBUG_MODE */
}

void DebugPrint(const char *msg, ...) {
#ifdef DRAGONWARE_DEBUG_MODE
        char    buf[256] = {0};
        va_list ap;
        va_start(ap, msg);
        vsnprintf(buf, sizeof(buf), msg, ap);
        va_end(ap);

        /* Print the prefix first implicitly */
        Size prefixlen = strlen(BOOTMGR_BRANDING_STRING);
        for (Size i = 0; i < prefixlen; i++) WriteSerialChar(BOOTMGR_BRANDING_STRING[i]);

        Size len = strlen(buf);
        if (len >= 256) len = 256; /* Make sure it's truncated so we don't overread */
        for (Size i = 0; i < len; i++) WriteSerialChar(buf[i]);

        WriteSerialChar('\n');
#else
        UnusedParameter(msg);
#endif /* DRAGONWARE_DEBUG_MODE */
}
