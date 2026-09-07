/**********************************************************************
 * FILE: section_object.c
 * PURPOSE: Section object wrappers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <object.h>
#include <stdint.h>

Handle RequestMemorySection(u32 n_pages, SectionPermissions permissions) {
        Handle section_object = CreateObject(NullPointer, OBJ_SECTION, 0);
        if (section_object < 0) return section_object;

        UserSectionDescriptor descriptor = {
                .needed_pages = n_pages,
                .perms = permissions
        };
        Status s = InvokeObject(section_object, SECTION_REQUEST, &descriptor);
        if (s != STATUS_OK) goto bad;
        return section_object;
bad:
        DeleteObject(section_object);
        return -1;
}

Status MapMemorySection(Handle section, void **base) {
        uintptr_t baseaddr;
        Status s = InvokeObject(section, SECTION_MAP, &baseaddr);
        if (s == STATUS_OK) *base = (void*)baseaddr;
        return s;
}
