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
#include <mmutils.h>

#include "ddk/ia32/irq.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/vmm.h"
#include "iomgr/class.h"
#include "iomgr/devmgr.h"
#include "iomgr/node.h"
#include "iomgr/object.h"
#include "iomgr/port.h"
#include "iomgr/section.h"
#include "iomgr/thread.h"
#include "sched/schedule.h"
#include "syscall/usercopy.h"
#include "task/process.h"
#include "task/task.h"
#include "video/output.h"

typedef struct [[gnu::packed]] _DeviceMapDescriptor {
        u32 irq;
        u64 mmio_addr;
        u64 mmio_len;
        struct [[gnu::packed]] {
                u32 width;
                u32 height;
                u32 bpp;
                u32 stride;
        } fb;
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

                        if (CopyFromUser(path_real, path, pathsize) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;
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

                                DeviceMapDescriptor descr = {.irq       = 0, /* TODO */
                                                             .mmio_addr = dev->attr.mmio_addr,
                                                             .mmio_len  = dev->attr.mmio_len};
                                if (SupportsClass(dev, DEVCLASS_FRAMEBUFFER)) {
                                        /* Wheeewh wheeewh wheeewh */
                                        FramebufferInformation fbi =
                                                dev->devtable.ddo->framebuffer
                                                        .GetFramebufferInformation(
                                                                dev->private_state);
                                        descr.fb.width  = fbi.width;
                                        descr.fb.height = fbi.height;
                                        descr.fb.bpp    = fbi.bpp;
                                        descr.fb.stride = fbi.stride;
                                } else {
                                        /* Better zero it out to be sure */
                                        kzeromem(&descr.fb, sizeof(descr.fb));
                                }
                                if (CopyToUser(devdescr, &descr, sizeof(DeviceMapDescriptor)) !=
                                    STATUS_OK)
                                        return STATUS_BAD_ARGUMENT;

                                dev->attr.claimed = true;
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
                        if (!(current->owner->flags & (PROC_C_SERVER | PROC_C_IRQ_DISPATCH)))
                                return STATUS_UNSUPPORTED;
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

static Status HandleThreadObjectRequest(int handle, Object *obj, ThreadObjectOp op, void *arg) {
        UnusedParameter(handle);
        switch (op) {
                case THREAD_CREATE: {
                        /* already created thread, avoid allocating twice */
                        if (obj->data) return STATUS_BAD_ARGUMENT;
                        UserThreadData data;
                        if (CopyFromUser(&data, arg, sizeof(UserThreadData)) != STATUS_OK)
                                return STATUS_BAD_ARGUMENT;

                        if (!data.stack || !data.entry) return STATUS_BAD_ARGUMENT;

                        obj->data = CreateThread(data.entry, data.stack, data.extra_data);
                        return (obj->data) ? STATUS_OK : STATUS_BAD;
                }
                case THREAD_RUN: {
                        if (obj && obj->data) {
                                RunThread(obj);
                                return STATUS_OK;
                        } else
                                return STATUS_BAD_ARGUMENT;
                }
                default:
                        return STATUS_BAD_ARGUMENT;
        }
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
                case OBJ_THREAD:
                        return HandleThreadObjectRequest(handle, target, op, argptr);
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

Status _DWTranslateHandle(ProcessID of, int handle_of, int *save) {
        if (!of) return STATUS_BAD_ARGUMENT;
        if (handle_of >= MAX_OBJ_PER_PROCESS || handle_of < 0) return STATUS_BAD_ARGUMENT;

        Process *origin = FindProcessByID(of);
        Process *this   = GetCurrentExecutionThread()->owner;
        if (origin == this) goto same;
        if (!origin) return STATUS_NOT_FOUND;

        Object *original = origin->handles.objlist[handle_of];

        Status copy_status = STATUS_OK;
        if (original) {
                HandleTable *target_table = &this->handles;
                int          new_hdl      = -1;

                /* Check if the process already has a handle to this port to avoid
                 * duplication. */
                for (int i = 0; i < MAX_OBJ_PER_PROCESS; i++) {
                        /* If the server already has a handle to this process, no need to
                         * allocate a new one - Just reuse the existing one. */
                        if ((target_table->valid_bitmap & (1 << i)) &&
                            target_table->objlist[i] == original) {
                                new_hdl = i;
                                break;
                        }
                }

                if (new_hdl == -1) {
                        new_hdl = AppendToHandleTable(target_table, original);
                        if (new_hdl < 0) return STATUS_OUT_OF_MEMORY;
                        /* avoid a use after free if the original process tries to delete this
                         * object*/
                        else
                                original->refcnt++;
                }
                copy_status = CopyToUser(save, &new_hdl, sizeof(int));
                if (copy_status != STATUS_OK) {
                        DeleteFromHandleTable(target_table, new_hdl);
                        original->refcnt--; /* Safe to do so here, because we incremented this
                                               above, and nothing interrupts us in between */
                }
        } else
                return STATUS_NOT_FOUND;

        return copy_status;

same:
        /* yes, I am intentionally not checking if the handle points to anything valid or
         * not, because this is the concern of the caller, not the kernel. The kernel won't
         * access any data. */
        return CopyToUser(save, &handle_of, sizeof(int));
}
