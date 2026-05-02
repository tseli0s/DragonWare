/**********************************************************************
 * FILE: object.c
 * PURPOSE: Kernel object management implementation
 * PROJECT: DragonWare Kernel
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

#include "object.h"

#include <bits.h>
#include <kmalloc.h>
#include <ktypes.h>
#include <macros.h>

#include "iomgr/port.h"
#include "iomgr/section.h"

Object *AllocateObject(const char *name, ObjectType type, u32 permissions) {
        UnusedParameter(permissions); /* TODO */

        Object *obj = kmalloc(sizeof(Object));
        if (!obj) return NullPointer;

        obj->name        = name;
        obj->type        = type;
        obj->data        = NullPointer;
        obj->permissions = 0;
        obj->refcnt      = 1;

        return obj;
}

void DeleteObject(Object *obj) {
        /* trying to avoid integer underflows by accident */
        if (obj->refcnt > 1) {
                obj->refcnt--;
                return;
        }

        if (obj->refcnt > 0) obj->refcnt = 0;
        switch (obj->type) {
                case OBJ_PORT:
                        DeletePort((Port *)obj->data);
                        break;
                case OBJ_SECTION:
                        DeleteSection((Section *)obj->data);
                        break;
                case OBJ_DEVICE: /* The device manager should not have nodes deleted. */
                case OBJ_UNKNOWN:
                default:
                        break;
        }
        kfree(obj);
}
int AppendToHandleTable(HandleTable *table, Object *obj) {
        u32 free_mask = ~table->valid_bitmap;
        if (free_mask == 0) return -1;

        int index = __builtin_ctz(free_mask);

        SET_BIT(table->valid_bitmap, index);
        table->objlist[index] = obj;

        return index;
}

Object *DeleteFromHandleTable(HandleTable *table, int index) {
        Object *target = table->objlist[index];
        CLEAR_BIT(table->valid_bitmap, index);
        table->objlist[index] = NullPointer;

        return target;
}
