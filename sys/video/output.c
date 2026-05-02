/**********************************************************************
 * FILE: output.c
 * PURPOSE: Kernel-mode output access for DragonWare
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "output.h"

#include <kmalloc.h>
#include <ktypes.h>
#include <log.h>
#include <panic.h>

#include "ddk/ia32/vmm.h"
#include "iomgr/class.h"
#include "iomgr/node.h"
#include "macros.h"

static OutputNode *list    = NullPointer;
static OutputNode *current = NullPointer;

OutputNode *GetAllOutputsKnown(void) { return list; }

void AddNewOutputDevice(DeviceManagerNode *dev) {
        if (!SupportsClass(dev, DEVCLASS_CONSOLE) && !SupportsClass(dev, DEVCLASS_FRAMEBUFFER) &&
            !SupportsClass(dev, DEVCLASS_UART))
                return;
        if (!list) {
                list = kzalloc(sizeof(OutputNode));
                if (!list) return;

                list->node = dev;
                list->next = NullPointer;
        } else {
                OutputNode *iter = list;
                while (iter->next) iter = iter->next;
                iter->next = kzalloc(sizeof(OutputNode));
                if (!iter->next) return;

                iter->next->node = dev;
                iter->next->next = NullPointer;
        }
        if (!current) current = list;
}

Status RemoveKernelOutput(DeviceManagerNode *node) {
        if (!list || !current) return STATUS_NOT_FOUND;
        OutputNode **curr = &list;
        while (*curr) {
                OutputNode *entry = *curr;
                if (entry->node == node) {
                        /* Unmap any MMIO the kernel might've been using before. mmio_len must be
                         * page-aligned manually, we don't know whether it is well aligned or not.
                         */
                        Size n_pages_to_unmap = pagealign(node->attr.mmio_len) / PAGE_SIZE;
                        for (Size i = 0; i < n_pages_to_unmap; i++) {
                                const uintptr_t curraddr =
                                        (u32)((u32)node->attr.kernel_mapped_addr + (i * PAGE_SIZE));
                                UnmapSinglePage(curraddr);
                        }
                        *curr = entry->next;
                        /* Make sure if it is the output device used by the kernel, we automatically
                         * switch to some fallback. */
                        if (node == current->node) current = current->next ? current->next : list;
                        return STATUS_OK;
                }
                curr = &entry->next;
        }
        return STATUS_NOT_FOUND;
}

void SelectPrimaryOutputDevice(OutputNode *node) { current = node; }

OutputNode *GetPrimaryOutputDevice(void) { return current; }
