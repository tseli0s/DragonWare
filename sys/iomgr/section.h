/**********************************************************************
 * FILE: section.h
 * PURPOSE: Section object exports and helpers
 * PROJECT: DragonWare Kernel
 * DATE: 05-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* How many bytes of memory can a section occupy. This is to save memory on each section object,
 * because a section object internally also holds address to physical frames. So for 120 frames,
 * each object must hold 256 bytes of raw addresses. */
#define MAX_SECTION_FRAMES (120)

#include <ktypes.h>

#include "task/process.h"

/**
 * @brief A virtual address space section.
 * @details Sections in DragonWare describe a region in a virtual address space that can be used,
 * therefore implementing heaps and shared memory. A section does not describe where that section is
 * or to what physical memory is it mapped to - It states that "There is a range of pages that can
 * be written to"
 */
typedef struct _Section {
        ProcessID address_space; /* Which process' address space has this section mapped */
        Size      n_pages;       /* Amount of pages in this section */
        u32 permissions;  /* Permissions of the section (Whether it's writeable, cacheable, ...) */
        uintptr_t vmbase; /* Where the section starts in virtual memory, used to unmap it later. */
        uintptr_t physframes[MAX_SECTION_FRAMES]; /* Array of frame addresses that are mapped to the
                                                     section's virtual addresses. */
} Section;

/** @brief Flags describing the permissions of a single section. */
typedef enum _SectionPermissions : unsigned long {
        SECTION_NONE      = 0x00, /** << Nothing allowed on */
        SECTION_WRITEABLE = 0x01, /** << Page can be written upon. */
        SECTION_SHAREABLE = 0x02, /** << Section may be shared with another process */
        SECTION_CACHEABLE = 0x04, /** << Page writes may not be cached by hardware and should take
                                     effect immediately. */
} SectionPermissions;

/** @brief A descriptor for a section request passed from user programs to the kernel. */
typedef struct [[gnu::packed]] _UserSectionDescriptor {
        Size needed_pages; /** << Amount of needed pages. Must be above zero and less than @ref
                              MAX_SECTION_FRAMES */
        SectionPermissions perms; /** << Permissions bitfield. See @ref SectionPermissions */
} UserSectionDescriptor;

/**
 * @brief Allocates a section object of @p needed_pages
 * @param needed_pages Amount of pages to allocate in this section.
 * @param permissions Bitfield of permissions that describe the permissions of this section. See
 * @ref SectionPermissions
 * @note The actual pages are not allocated yet - The section object must be invoked for the kernel
 * to allocate and map the pages of it.
 * @returns The @ref Section object allocated.
 */
Section *AllocateSection(Size needed_pages, SectionPermissions permissions);

/**
 * @brief Maps a section in the current virtual address space.
 * @note Since sections are meant exclusively for user programs (The kernel has its own heap and
 * data structures), the return value will never exceed @ref KERNEL_VM_BASE and the mapped pages
 * will always be user-readable.
 * @param[in] section The section to map. Must not be NullPointer.
 * @param copy_on_write Unused, for future expansion only.
 * @warning This function will run in the CURRENT address space, ie. Whatever process is running
 * when the function is called.
 * @returns The starting address of the section in the address space, or 0 if it cannot be mapped
 * (eg. Not enough physical memory)
 */
uintptr_t MapSection(Section *section, Bool copy_on_write);

/**
 * @brief Deletes a single section and unmaps it from memory, also freeing any data it may contain.
 * @param[in] section The section to delete.
 */
void DeleteSection(Section *section);
