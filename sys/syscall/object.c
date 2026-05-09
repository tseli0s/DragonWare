/**********************************************************************
 * FILE: object.c
 * PURPOSE: Kernel object system call API implementation
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "object.h"

#include <kmalloc.h>
#include <kstring.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>

#include "ddk/ia32/irq.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "iomgr/object.h"
#include "iomgr/port.h"
#include "iomgr/section.h"
#include "sched/schedule.h"
#include "syscall/usercopy.h"
#include "task/process.h"
#include "task/task.h"
#include "video/output.h"

typedef struct [[gnu::packed]] _DeviceMapDescriptor {
        u32 irq;
        u64 mmio_addr;
        u64 mmio_len;
} DeviceMapDescriptor;

static Status HandleDeviceObjectRequest(Object *obj, DeviceObjectOp op, void *arg) {
        Process           *p_current = GetCurrentExecutionThread()->owner;
        DeviceManagerNode *dev       = obj->data;

        if (!(p_current->flags & PROC_C_SERVER)) return STATUS_UNSUPPORTED;

        switch (op) {
                case DEVICE_GET: {
                        if (!ADDRESS_IS_MAPPED(arg)) return STATUS_BAD_ARGUMENT;

                        const char *path      = arg;
                        Size        pathsize  = strnlen(path, MAX_DEVICE_NODE_NAME * 16) + 1;
                        char       *path_real = kmalloc(pathsize);
                        if (!path_real) return STATUS_OUT_OF_MEMORY;

                        CopyFromUser(path_real, path, pathsize);
                        path_real[pathsize - 1] = '\0';

                        obj->data = GetDeviceFromPath(path_real);
                        if (!obj->data) {
                                kfree(path_real);
                                return STATUS_NOT_FOUND;
                        }
                        kfree(path_real);
                        break;
                }
                case DEVICE_CLAIM: {
                        /* WARNING: User pointer below */
                        DeviceMapDescriptor *devdescr = arg;
                        if (!ADDRESS_IS_MAPPED(arg)) return STATUS_BAD_ARGUMENT;

                        if (likely(dev)) {
                                if (unlikely(dev->attr.claimed)) return STATUS_BAD;

                                dev->attr.claimed         = true;
                                DeviceMapDescriptor descr = {.irq       = 0, /* TODO */
                                                             .mmio_addr = dev->attr.mmio_addr,
                                                             .mmio_len  = dev->attr.mmio_len};
                                if (CopyToUser(devdescr, &descr, sizeof(DeviceMapDescriptor)) !=
                                    STATUS_OK)
                                        return STATUS_BAD_ARGUMENT;

                                LogMessage(LOG_INFO, "Device '%s' at %p claimed by process %d",
                                           dev->attr.name, dev, p_current->pid);
                                if (SupportsClass(dev, DEVCLASS_FRAMEBUFFER) ||
                                    SupportsClass(dev, DEVCLASS_CONSOLE)) {
                                        RemoveKernelOutput(dev);
                                        FlushTLB();
                                }
                        } else
                                return STATUS_NOT_FOUND;

                        break;
                }
                case DEVICE_MAP: {
                        if (!dev->attr.claimed) return STATUS_RETRY;
                        if (dev->attr.mmio_addr >= 0xFFFFFFFF) return STATUS_UNSUPPORTED;

                        uintptr_t mapaddr     = (uintptr_t)arg;
                        Size      n_pages_map = pagealign(dev->attr.mmio_len) / PAGE_SIZE;

                        if (unlikely(mapaddr >= KERNEL_VM_BASE ||
                                     dev->attr.mmio_len > KERNEL_VM_BASE - mapaddr)) {
                                return STATUS_BAD_ARGUMENT;
                        }

                        Size mapped =
                                MapMemoryRange((uintptr_t)dev->attr.mmio_addr, (uintptr_t)mapaddr,
                                               PAGE_PRESENT | PAGE_RW | PAGE_USER |
                                                       PAGE_WRITETHROUGH | PAGE_CACHE_DISABLED,
                                               n_pages_map);
                        if (unlikely(mapped != n_pages_map)) return STATUS_BAD;
                        break;
                }
                default:
                        return STATUS_BAD_ARGUMENT;
        }
        return STATUS_OK;
}

static Status HandlePortObjectRequest(int handle, Object *obj, PortObjectOp op, void *arg) {
        Thread *current = GetCurrentExecutionThread();

        switch (op) {
                case PORT_CREATE: {
                        return CreatePort(obj->name, current, (Port **)&obj->data);
                }
                case PORT_OPEN: {
                        char name[MAX_PORT_NAME + 1];
                        if (CopyFromUser(name, arg, MAX_PORT_NAME) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;

                        name[MAX_PORT_NAME] = '\0';
                        Port *p             = FindPortByName(name);
                        if (!p) return STATUS_NOT_FOUND;

                        obj->data = p;
                        obj->refcnt++;
                        break;
                }
                case PORT_WAIT_AT: {
                        /* TODO */
                        break;
                }
                case PORT_BIND_IRQ: {
                        if (!(current->owner->flags & PROC_C_SERVER)) return STATUS_UNSUPPORTED;
                        IRQBindingDescriptor descr;
                        if (CopyFromUser(&descr, arg, sizeof(IRQBindingDescriptor)) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;
                        RegisterIRQSubscriber(descr.irq_no, handle);

                        break;
                }
                case PORT_ACK_IRQ: {
                        if (!(current->owner->flags & PROC_C_SERVER)) return STATUS_UNSUPPORTED;
                        IRQBindingDescriptor descr;
                        if (CopyFromUser(&descr, arg, sizeof(IRQBindingDescriptor)) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;

                        AcknowledgeUserIRQ(descr.irq_no, handle);
                        break;
                }
                default:
                        return STATUS_UNSUPPORTED;
        }

        return STATUS_OK;
}

static Status HandleSectionObjectRequest(int handle, Object *obj, SectionObjectOp op, void *arg) {
        UnusedParameter(handle);
        Process *current = GetCurrentExecutionThread()->owner;
        switch (op) {
                case SECTION_REQUEST: {
                        if (obj->data) {
                                /* Already allocated section, that's gonna cause a memory leak if we
                                 * don't return */
                                return STATUS_BAD;
                        }

                        UserSectionDescriptor descr;
                        if (CopyFromUser(&descr, arg, sizeof(UserSectionDescriptor)) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;

                        Section *section = AllocateSection(descr.needed_pages, descr.perms);
                        if (!section) return STATUS_OUT_OF_MEMORY;

                        section->address_space = current->pid;
                        obj->data              = section;
                        return STATUS_OK;
                }
                case SECTION_MAP:
                        if (!ADDRESS_IS_MAPPED(arg)) return STATUS_BAD_ARGUMENT;

                        uintptr_t base = MapSection(obj->data, false);
                        if (!base) return STATUS_BAD;
                        *(uintptr_t *)arg = base;
                        return STATUS_OK;
                case SECTION_SHARE:
                        return STATUS_UNSUPPORTED; /* TODO */
                default:
                        return STATUS_BAD_ARGUMENT;
        }
        return STATUS_OK;
}

int _DWCreateObject(const char *name, ObjectType type, u32 permissions) {
        UnusedParameter(permissions);

        char *name_real = NullPointer;
        if (name && ADDRESS_IS_MAPPED(name)) {
                Size strsize = strnlen(name, MAX_PORT_NAME) + 1;
                name_real    = kmalloc(strsize);
                if (!name_real) return -1;

                if (CopyFromUser(name_real, name, strsize) != STATUS_OK) {
                        kfree(name_real);
                        return -1;
                }
                name_real[strsize - 1] = '\0';
        }

        Object *obj = AllocateObject(name_real, type, 0);
        if (!obj) return -1;

        Process *curr = GetCurrentExecutionThread()->owner;
        return AppendToHandleTable(&curr->handles, obj);
}

[[gnu::hot]]
Status _DWInvokeObject(int handle, unsigned long op, void *argptr) {
        if (handle >= MAX_OBJ_PER_PROCESS || handle < 0) return STATUS_BAD_ARGUMENT;

        Process *current = GetCurrentExecutionThread()->owner;
        Object  *target  = current->handles.objlist[handle];
        if (!target) return STATUS_NOT_FOUND;

        switch (target->type) {
                case OBJ_UNKNOWN: {
                        /* Possibly kernel bug, so let the user know */
                        LogMessage(
                                LOG_WARNING,
                                "SYSCALL_INVOKE_OBJECT on a possibly invalid object (target->type "
                                "is OBJ_UNKNOWN, reserved to detect zeroed objects)");
                        return STATUS_BAD;
                }
                case OBJ_DEVICE:
                        return HandleDeviceObjectRequest(target, op, argptr);
                case OBJ_PORT:
                        return HandlePortObjectRequest(handle, target, op, argptr);
                case OBJ_SECTION:
                        return HandleSectionObjectRequest(handle, target, op, argptr);
                default:
                        return STATUS_BAD_ARGUMENT;
        }
        return STATUS_OK;
}

void _DWDeleteObject(int handle) {
        /* Intentionally ignore negative (invalid) handles. See base/vgacons/console.c near the
         * cleanup label. */
        if (handle < 0) return;

        Process *current = GetCurrentExecutionThread()->owner;
        Object  *target  = current->handles.objlist[handle];
        if (!target) return;

        /* The name is always kmalloc()ed into the kernel (see _DWCreateObject()) so this is safe to
         * do, in fact this must be done to avoid memory leaks */
        if (target->name) kfree((void *)target->name);

        DeleteFromHandleTable(&current->handles, handle);
        DeleteObject(target);
}
