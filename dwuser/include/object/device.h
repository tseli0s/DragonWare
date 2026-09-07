/**********************************************************************
 * FILE: device.h
 * PURPOSE: Device object helpers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kernelapi.h>
#include <kerneltypes.h>
#include <object.h>

static inline Status GetDevice(Handle devhandle, const char *devpath) {
        return InvokeObject(devhandle, DEVICE_GET, (void *)devpath);
}

static inline Status ClaimDevice(Handle devhandle, DeviceMapDescriptor *descriptor) {
        return InvokeObject(devhandle, DEVICE_CLAIM, &descriptor);
}

[[gnu::nonnull]]
static inline Status MapDeviceMMIO(Handle devhandle, void *addr) {
        return InvokeObject(devhandle, DEVICE_MAP, addr);
}
