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

#include "video/pixels.h"

/* Remember: A device may be able to do more than one thing. For example, the GPU is able to blit on
 * the screen as well as run custom code. So I've went with bitfields using  */

/**
 * @brief Returns whether a device supports a given operation based on its internal devtable class
 * flags
 */
#define SupportsClass(_dev, _class) (_dev->devtable.class & _class)

/* We can't use an enum; Their size is not guaranteed nor portable. Since we need 64 bits anyways,
 * let's typedef a u64 instead. That gives us 64 different capabilities per device (One bit per
 * capability)*/
typedef u64 DeviceClass;

#define DEVCLASS_UNKNOWN     (0x0)
#define DEVCLASS_FRAMEBUFFER (1ULL << 0)
#define DEVCLASS_CONSOLE     (1ULL << 1)
#define DEVCLASS_UART        (1ULL << 2)
#define DEVCLASS_CLOCK       (1ULL << 3)
#define DEVCLASS_IOMEDIA     (1ULL << 4)

typedef struct _FramebufferDeviceOps {
        void (*WriteSinglePixel)(void *privatedata, Size x, Size y, PixelColor color);
        void (*BlitRectangle)(void *privatedata, Size startx, Size starty, Size width, Size height,
                              PixelColor color);
        void (*Flush)(void *privatedata);
        void (*SetCurrentOutputColors)(void *privatedata, PixelColor fg, PixelColor bg);
        void (*ClearScreen)(void *privatedata);
} FramebufferDeviceOps;

typedef struct _ConsoleDeviceOps {
        void (*WriteSingleChar)(void *privatedata, char c);
        void (*SetTextAttributes)(void *privatedata, PixelColor background, PixelColor foreground);
        void (*ResetConsole)(void *privatedata);
        void (*DeleteSingleChar)(void *privatedata);
} ConsoleDeviceOps;

typedef struct _UARTDeviceOps {
        void (*WriteSingleChar)(void *privatedata, char c);
        Byte (*ReceiveByteFromSerial)(void *privatedata);
} UARTDeviceOps;
