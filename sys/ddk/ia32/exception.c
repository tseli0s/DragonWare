/**********************************************************************
 * FILE: exception.c
 * PURPOSE: Exception routines called from the InterruptServiceHandler function
 * PROJECT: DragonWare Kernel
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include "exception.h"

#include "gdt.h"
#include "interrupts.h"
#include "log.h"
#include "panic.h"
#include "power.h"
#include "sched/schedule.h"
#include "task/process.h"
#include "task/task.h"

extern volatile int NeedsResched;

static void _DWUserException(InterruptStackFrame *stack_frame) {
        switch ((ExceptionList)stack_frame->int_no) {
                case I_DIV0:
                case I_UD0:
                case I_GPF:
                case I_PF:
                default: {
                        u32     cr2  = GetFaultingAddress();
                        Thread *curr = GetCurrentExecutionThread();
                        curr->state  = THREAD_TERMINATED;
                        LogMessage(
                                LOG_ERROR,
                                "Process %u exception 0x%x, EFLAGS 0x%x, ERRCODE 0x%x EIP %p, ESP "
                                "%p CR2 0x%x",
                                curr->owner->pid, stack_frame->int_no, stack_frame->eflags,
                                stack_frame->err_code, stack_frame->eip, stack_frame->useresp, cr2);

                        /* TODO: If it's a server, it must be restarted, otherwise the system may be
                         * unable to *serve* applications that depended on it */
                        NeedsResched = 1;
                }
        }
}

void _DWExceptionHandler(InterruptStackFrame *stack_frame) {
        if (likely(stack_frame->cs == SEL_CODE_USER)) {
                _DWUserException(stack_frame);
                return;
        }
        switch ((ExceptionList)stack_frame->int_no) {
                case I_DIV0:
                        FatalErrorWithStackFrame(stack_frame, "Arithmetic division by zero!");
                        break;
                case I_DBG:
                        /* TODO: Kernel debugger */
                        LogMessage(LOG_DEBUG, "debug exception caught. halting kernel.");
                        StallMachine();
                case I_NMI:
                        LogMessage(LOG_INFO, "Non-maskable interrupt");
                        break;
                case I_BRK:
                        LogMessage(LOG_DEBUG, "Kernel breakpoint");
                        StallMachine();
                        break;
                case I_OVFW:
                        FatalErrorWithStackFrame(stack_frame, "Overflow exception occured");
                        break;
                case I_UD0:
                        FatalErrorWithStackFrame(stack_frame, "Invalid processor instruction");
                        break;
                case I_SBRE:
                case I_BUSY:
                        FatalErrorWithStackFrame(stack_frame, "Device or resource busy");
                        break;
                case I_DFAULT:
                        FatalErrorWithStackFrame(stack_frame, "Double fault occured");
                        break;
                case I_BTSS:
                        FatalErrorWithStackFrame(stack_frame, "Invalid TSS");
                        break;
                case I_SEGNP:
                        FatalErrorWithStackFrame(stack_frame, "Segment not present");
                        break;
                case I_SSF:
                        FatalErrorWithStackFrame(stack_frame, "Stack segment fault");
                        break;
                case I_GPF:
                        FatalErrorWithStackFrame(stack_frame,
                                                 "Protection Violation at 0x%x, code 0x%x, "
                                                 "(cs 0x%x, eflags 0x%x)",
                                                 stack_frame->eip, stack_frame->err_code,
                                                 stack_frame->cs, stack_frame->eflags);
                        break;
                case I_PF: {
                        u32 cr2 = GetFaultingAddress();
                        if (cr2 == 0x0)
                                FatalErrorWithStackFrame(
                                        stack_frame,
                                        "Attempted to dereference null "
                                        "pointer in supervisor mode at 0x%x, EFL: 0x%x",
                                        stack_frame->eip, stack_frame->eflags);
                        else
                                FatalErrorWithStackFrame(
                                        stack_frame,
                                        "Unresolved page fault exception in supervisor "
                                        "mode: Code "
                                        "0x%x, Address 0x%x, EFL: 0x%x, CR2 0x%x",
                                        stack_frame->err_code, stack_frame->eip,
                                        stack_frame->eflags, cr2);
                        break;
                }
                case I_FPE:
                        LogMessage(LOG_WARNING, "Bad floating point opcode");
                        break;
                case I_ALIGN:
                        FatalErrorWithStackFrame(stack_frame, "Alignment check exception");
                        break;
                case I_MCE:
                        FatalErrorWithStackFrame(stack_frame,
                                                 "Machine check exception -- Your hardware may be "
                                                 "faulty.");
                        break;
                default:
                        LogMessage(LOG_WARNING, "Unknown exception %d raised (Maybe fallthrough?)",
                                   stack_frame->int_no);
                        break;
        }
}
