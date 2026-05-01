/**********************************************************************
 * FILE: exception.h
 * PURPOSE: Exception routines called from the InterruptServiceHandler function
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define MAX_EXCEPTIONS (32)
#define EXCEPTION_BASE (0)

#include "interrupts.h"

typedef enum _ExceptionList {
        I_DIV0 = 0x0, /* Division by zero */
        I_DBG,        /* Debug exception */
        I_NMI,        /* Non-maskable interrupt (??) */
        I_BRK,        /* Breakpoint */
        I_OVFW,       /* Overflow */
        I_SBRE,       /* System bound range exceeded (Whatever that means) */
        I_UD0,        /* Invalid processor instruction */
        I_BUSY,       /* Device or resource is busy */
        I_DFAULT,     /* Double fault occurred */
        I_CPSO,       /* Coprocessor segment overrun */
        I_BTSS,       /* Bad task state segment */
        I_SEGNP,      /* Segment not present */
        I_SSF,        /* Stack segment fault */
        I_GPF,        /* General protection fault */
        I_PF,         /* Page fault (Recoverable) */
        I_FPE,        /* Floating point exception */
        I_ALIGN,      /* Alignment check exception*/
        I_MCE,        /* Machine check exception */
} ExceptionList;

/**
 * @brief Decode the virtual address that caused a page fault by reading the @code cr2 @endcode
 * register. This may be 0 - If so, this almost certainly means a null pointer was dereferenced.
 * @return The value of the %cr2 register in IA32, automatically set by the processor when a page
 * fault exception is raised.
 */
static inline u32 GetFaultingAddress(void) {
        u32 cr2;
        __asm__ volatile("movl %%cr2, %0" : "=r"(cr2)::);
        return cr2;
}

/**
 * @brief Exception handler for DragonWare. It reads the interrupt code pushed by an
 * InterruptServiceRoutine and decides what happens with the interrupt.
 * @param stack_frame The original stack frame pushed by an InterruptServiceRoutine
 * @warning If the exception was critical, this function may not return. Do not depend on its
 * behaviour.
 */
void _DWExceptionHandler(InterruptStackFrame *stack_frame);
