/**********************************************************************
 * FILE: wfi.c
 * PURPOSE: Main kernel loop (Waiting for interrupts and putting the machine in a low power state
 * PROJECT: DragonWare Kernel
 * DATE: 01-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "wfi.h"

#include <ktypes.h>
#include <macros.h>

#include "ddk/ia32/interrupts.h"

[[noreturn]]
void WaitForInterrupts(void) {
#ifdef __i386__
        /* We are now ready to start receiving interrupts. Obviously this depends on ArchInit() to
         * set up the processor-specific structures that allow interrupts to be handled.*/
        EnableInterrupts();
        while (true) {
                /*
                 * hlt: Halts the processor until the next external interrupt, NMI, SMI or reset
                 * occurs. (Halts means that the processor won't execute more instructions until the
                 * next interrupt and may enter a low power state. */
                __asm__ volatile("hlt");
        }

        /* If we somehow escaped the loop, make sure that interrupts are disabled. */
        DisableInterrupts();
        __builtin_unreachable();
#else
#error Unsure how to implement WaitForInterrupts for this target.
#endif /* __i386__ */
}
