/**********************************************************************
 * FILE: main.c
 * PURPOSE: Framebuffer microkernel server for DragonWare
 * PROJECT: DragonWare Base System
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>

#define EXPECTED_FRAMEBUFFER_ADDR ((void *)(0x80000000))

int main(void) {
        int                 handle            = _DWCreateObject(NullPointer, OBJ_DEVICE, 0);
        DeviceMapDescriptor device_descriptor = {0};

        if (handle < 0) {
                _DWklog(LOG_ERROR,
                        "Unable to create device object (Required to claim framebuffer)");
                return -1;
        }
        if (_DWInvokeObject(handle, DEVICE_GET, "/Devices/Kernel Framebuffer") != STATUS_OK) {
                _DWklog(LOG_ERROR,
                        "Unable to find /Devices/Kernel Framebuffer in the kernel device tree.");
                return -1;
        }
        if (_DWInvokeObject(handle, DEVICE_CLAIM, &device_descriptor) != STATUS_OK) {
                _DWklog(LOG_ERROR, "Unable to claim /Devices/Kernel Framebuffer for server use");
                return -1;
        }

        if (_DWInvokeObject(handle, DEVICE_MAP, EXPECTED_FRAMEBUFFER_ADDR) != STATUS_OK) {
                _DWklog(LOG_ERROR,
                        "Framebuffer device could not be mapped to the framebuffer "
                        "server.");
                return -1;
        }

        u32 *buf = EXPECTED_FRAMEBUFFER_ADDR;
        for (unsigned int i = 0; i < (device_descriptor.mmio_len / 4) - 1; i++) buf[i] = 0x00000000;

        while (1) {
                Message m;
                if (_DWIPCReceive(0, &m) == 0) {
                        _DWklog(LOG_DEBUG,
                                "Message received to framebuffer server. This is just a "
                                "placeholder.");
                }
        }
        return 0;
}
