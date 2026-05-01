/**********************************************************************
 * FILE: main.c
 * PURPOSE: I/O port privilege testing binary for DragonWare
 * PROJECT: DragonWare Base System
 * DATE: 03-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>

int main(void) {
        _DWklog(LOG_INFO, "Attempting to raise I/O privileges through system call 5...");
        if (_DWRaiseIOPL() != 0)
                _DWklog(LOG_ERROR,
                        "Kernel rejected the IOPL raise request, process possibly doesn't "
                        "have the capability for it...");
        _DWklog(LOG_INFO, "Now testing I/O...");
        __asm__ volatile("inb $0x80");

        /*
         * Dear source code reader. Currently reading this to get inspired, understand more about
         * OSes, or just out of curiosity... Know that my dumb ass spent two whole weeks, turning
         * the kernel inside out, trying to figure out why did the PIT randomly stop firing in
         * Bochs. I kid you not, I thought I had totally fucked over my kernel. I ran it through
         * five different debuggers, three emulators, two different branches and even forced myself
         * to rewrite parts of the IRQ logic by hand. Like an absolute fucking moron. The source of
         * the bug was that the line below was:
         * __asm__ volatile("outb %al, $0x40"); <--------- 0x40!!
         *
         * Notice that 0x40? That's the PIT port. You know, the thing that fires every millisecond
         * or two or five or whatever you configure it, to tell your kernel "hey moron, perhaps
         * decrement the quantum of the thread. A tick passed". And this program happily sent random
         * one byte to the PIT (the PIT expects two bytes for the frequency), the PIT deadlocked
         * waiting for the next byte, and we got a system that wouldn't preempt threads.
         *
         * The lesson here, dear reader, is that you NEVER, EVER touch ports like the one, you know,
         * RUNNING THE ENTIRE FUCKING OPERATING SYSTEM. The other lesson, dear reader, is that you
         * NEVER, EVER give arbitrary programs the ability to, you know, STALL THE ENTIRE FUCKING
         * OPERATING SYSTEM. It's a necessity to allow this, despite the massive insecurity I just
         * proved to myself, because DragonWare runs most of the services of the operating system as
         * regular preemptive programs.
         *
         * The only thing I'm angry about is that this took away two whole weeks of development from
         * DragonWare. Two weeks that could've really made a lot of progress in much needed areas.
         * Please, dear reader, never be a dumb fucking idiot like me. Or at the very least
         * remember, port 0x40 is the fucking timer that tells your scheduler when to switch the
         * threads.
         */
        __asm__ volatile("outb %al, $0x80");

        _DWklog(LOG_INFO, "Read/write operations to ports succeeded. Quitting.");
        return 0;
}
