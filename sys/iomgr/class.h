/**********************************************************************
 * FILE: class.h
 * PURPOSE: Device Manager node classes
 * PROJECT: DragonWare Kernel
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <ktypes.h>

/**
 * @brief Returns whether a device supports a given operation based on its internal devtable class
 * flags
 */
#define SupportsClass(_dev, _class) ((_dev)->devtable.class & (_class))

typedef enum _DeviceClass
        : u64 { DEVCLASS_UNKNOWN     = 0x0,
                DEVCLASS_FRAMEBUFFER = 0x1,
                DEVCLASS_CONSOLE     = 0x2,
                DEVCLASS_UART        = 0x4 } DeviceClass;

typedef struct _FramebufferInformation {
        u32 width;
        u32 height;
        u32 bpp;
        u32 stride;
} FramebufferInformation;

typedef struct _FramebufferDeviceOps {
        void                   (*WriteSinglePixel)(void *privatedata, Size x, Size y, int bw);
        void                   (*ClearScreen)(void *privatedata);
        FramebufferInformation (*GetFramebufferInformation)(void *privatedata);
} FramebufferDeviceOps;

typedef struct _ConsoleDeviceOps {
        void (*WriteSingleChar)(void *privatedata, char c);
        void (*ResetConsole)(void *privatedata);
        void (*DeleteSingleChar)(void *privatedata);
} ConsoleDeviceOps;

typedef struct _UARTDeviceOps {
        void (*WriteSingleChar)(void *privatedata, char c);
} UARTDeviceOps;
