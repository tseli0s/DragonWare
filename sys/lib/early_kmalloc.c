/**********************************************************************
 * FILE: early_kmalloc.c
 * PURPOSE: Static memory allocation for DragonWare, used to bring up the actual allocator later
 * PROJECT: DragonWare Kernel
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#define AllocateStaticMemory_SIZE (2048)
#define CHUNK_SIZE                (32)

#include "early_kmalloc.h"

#include <ktypes.h>

#include "lib/assert.h"

static u8   buffer[AllocateStaticMemory_SIZE];
static u64  allocated        = 0x00000000; /* Perfectly matches AllocateStaticMemory_SIZE */
static Size remaining_memory = AllocateStaticMemory_SIZE;

void *AllocateStaticMemory(Size size) {
        if (!size) return NullPointer;

        kassert(size <= remaining_memory);
        kassert(remaining_memory > 0);

        Size chunks_needed = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
        for (Size i = 0; i < 64; i++) {
                Size free_count = 0;
                for (Size j = 0; j < chunks_needed && (i + j) < 64; j++) {
                        if (!(allocated & (1ULL << (i + j)))) {
                                free_count++;
                        } else
                                break;
                }

                if (free_count == chunks_needed) {
                        for (Size j = 0; j < chunks_needed; j++) {
                                allocated |= (1ULL << (i + j));
                        }

                        remaining_memory -= chunks_needed * CHUNK_SIZE;
                        return &buffer[i * CHUNK_SIZE];
                }
        }
        return NullPointer;
}
