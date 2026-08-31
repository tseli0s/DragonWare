/**********************************************************************
 * FILE: status.c
 * PURPOSE: Status type helpers implementation
 * PROJECT: DragonWare User Library
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <kerneltypes.h>
#include <macros.h>

/*
 * XXX this should be expanded every time we add a new Status code
 * Also, status codes are negative (There were good reasons when I decided that) which is why I'm
 * flipping the sign in every status code below.
 */
static const char *strings[] = {
        /* Yeah we don't need the negative sign here in STATUS_OK i know it just looks better when
           the whole thing is aligned evenly */
        [-STATUS_OK]            = "Success",
        [-STATUS_BAD]           = "Failure",
        [-STATUS_OUT_OF_MEMORY] = "Out of memory",
        [-STATUS_BAD_ARGUMENT]  = "Bad call argument",
        [-STATUS_RETRY]         = "Retry again later",
        [-STATUS_UNSUPPORTED]   = "Unsupported operation",
        [-STATUS_NOT_FOUND]     = "Resource not found",
        [-STATUS_OUT_OF_BOUNDS] = "Out of bounds request",
        [-STATUS_TIMEOUT]       = "Resource timeout",
        [-STATUS_NO_ENDPOINT]   = "Message endpoint unreachable",
        [-STATUS_BAD_SYSCALL]   = "Bad system call number",
        [-STATUS_MSGQUEUE_FULL] = "Recipient message queue is full",
};

#define UNMATCHED_STATUS_STRING ("Unknown")

const char *StringifyStatus(Status status_code) {
        if (status_code > 0) return UNMATCHED_STATUS_STRING;
        if ((unsigned)(-status_code) > arraysize(strings)) return UNMATCHED_STATUS_STRING;

        return strings[-status_code];
}
