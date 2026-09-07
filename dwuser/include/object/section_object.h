/**********************************************************************
 * FILE: section_object.h
 * PURPOSE: Section object helpers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kernelapi.h>
#include <kerneltypes.h>
#include <object.h>

/**
 * @brief Request a memory section of size at least @p n_pages to be allocated for the calling
 * process. The memory section is not mapped, only prepared.
 * @param n_pages Amount of pages this section must allocate. For the page size, see @ref
 * _DWSystemQuery with the @ref SQ_PAGE_SIZE value.
 * @param permissions Permissions of this memory section. Documentation can be found on @ref
 * SectionPermissions
 * @returns Handle to the referencing object of this section created internally, or -1 if the object
 * could not be allocated or the kernel rejected the request.
 */
Handle RequestMemorySection(u32 n_pages, SectionPermissions permissions);

/**
 * @brief Maps the section referenced to by object handle @p section into the current address space
 * and returns in @p base the starting address of the section.
 * @param section Handle to the section object. See @ref CreateObject
 * @param[out] base Non-NULL pointer to a pointer where the starting address of this section will be
 * written.
 * @returns A @ref Status value, where, if not STATUS_OK, may be: STATUS_BAD_ARGUMENT if @p base is
 * an invalid/untrusted pointer, or STATUS_BAD if mapping the section failed internally (eg. No free
 * virtual memory for the calling process)
 */
[[gnu::nonnull]]
Status MapMemorySection(Handle section, void **base);
