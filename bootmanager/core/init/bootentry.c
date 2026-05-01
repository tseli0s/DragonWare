/**********************************************************************
 * FILE: bootentry.c
 * PURPOSE: Boot entry support abstraction and helpers
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "bootentry.h"

#include <mmutils.h>
#include <power.h>

#include "ktypes.h"

static BootEntry entries[MAX_BOOT_ENTRIES] = {0};
static Size      num_entries               = 0;
static Size      selected_index            = 0;

Status AddEntry(const char *name, unsigned int index, void (*SelectCallback)(void)) {
        if (index >= MAX_BOOT_ENTRIES) return STATUS_OUT_OF_BOUNDS;
        if (!SelectCallback) return STATUS_BAD_ARGUMENT;
        entries[index] = (BootEntry){.name = name, .SelectCallback = SelectCallback};
        if (index >= num_entries) num_entries = index + 1;

        return STATUS_OK;
}

void UpdateIndex(unsigned int new) {
        if (inrange(new, 0, ((unsigned int)num_entries) - 1)) selected_index = new;
}

Size GetCurrentIndex(void) { return selected_index; }

void EntrySelected() {
        BootEntry *be = &entries[selected_index];
        if (!be->name) return;
        if (be->name[0] == 0) return;
        if (be->SelectCallback) be->SelectCallback();
}

BootEntry *GetAllBootEntries(Size *n_entries) {
        *n_entries = num_entries;
        return entries;
}
