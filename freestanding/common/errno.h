/**********************************************************************
 * FILE: errno.h
 * PURPOSE: Definition of error code constants
 * PROJECT: DragonWare Freestanding Library
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#define E_NONE     (0x0) /* No error */
#define E_NOSYS    (0x1) /* No such system call */
#define E_DENIED   (0x2) /* Access to resource denied */
#define E_BUSY     (0x3) /* Device or resource is busy */
#define E_NOOUT    (0x4) /* No display output */
#define E_NOTFOUND (0x5) /* File or directory not found */
#define E_INVALID  (0x6) /* Invalid argument given */
#define E_NOMEM    (0x7) /* Host has ran out of memory */
#define E_IO       (0x8) /* Input/Output error */
#define E_NODEV    (0x9) /* Device not present */
#define E_NOSPACE  (0xA) /* No space left for the operation */
#define E_BROKEN   (0xB) /* Unreliable communication with the process */
#define E_NOTTABLE (0xC) /* Host is not a table */
#define E_AGAIN    (0xD) /* Try again */
