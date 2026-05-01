/**********************************************************************
 * FILE: frame.c
 * PURPOSE: Physical memory allocating and freeing based on the kernel implementation
 * PROJECT: DragonWare Boot Manager
 * DATE: 02-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "frame.h"

#include <mmutils.h>

/* We will simply reserve 0x35000-0x80000 for the bootloader's heap. */
#define HEAP_START (0x35000)
#define HEAP_END   (0x80000)

typedef struct _PhysicalPool {
        uintptr_t frame_start; /* mem_start / PAGE_SIZE */
        Size      n_frames;    /* mem_len / PAGE_SIZE */
        uintptr_t free_list;   /* First freed frame */
} PhysicalPool;

static PhysicalPool pool = {
        .frame_start = HEAP_START / FRAME_SIZE,
        .n_frames    = (HEAP_END - HEAP_START) / FRAME_SIZE,
        .free_list   = 0,
};

void InitFrameManager(void) {
        pool.n_frames    = (HEAP_END - HEAP_START) / FRAME_SIZE;
        pool.frame_start = HEAP_START / FRAME_SIZE;
        pool.free_list   = 0;
}

uintptr_t AllocateFrame(void) {
        uintptr_t ptr = 0;

        if (pool.free_list != 0) {
                ptr            = pool.free_list;
                pool.free_list = *(uintptr_t*)ptr;
        } else if (pool.n_frames > 0) {
                ptr = pool.frame_start * FRAME_SIZE;
                pool.frame_start++;
                pool.n_frames--;
        }

        if (ptr) kzeromem((void*)ptr, FRAME_SIZE);
        return ptr;
}

void FreeFrame(uintptr_t frameaddr) {
        if (frameaddr == 0) return;

        *(uintptr_t*)frameaddr = pool.free_list;
        pool.free_list         = frameaddr;
}
