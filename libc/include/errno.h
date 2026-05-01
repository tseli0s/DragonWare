/**********************************************************************
 * FILE: errno.h
 * PURPOSE: errno code support for DragonWare libc
 * PROJECT: DragonWare C Library
 * DATE: 11-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "dlibc_common.h"

DLC_BEGIN_DECLS

extern volatile int errno;
extern int          __get_errno(); /* might be useful in the future */

#define EPERM           0x01 /* Operation not permitted */
#define ENOENT          0x02 /* No such file or directory */
#define ESRCH           0x03 /* No such process */
#define EINTR           0x04 /* Interrupted system call */
#define EIO             0x05 /* Input/output error */
#define ENXIO           0x06 /* No such device or address */
#define E2BIG           0x07 /* Argument list too long */
#define ENOEXEC         0x08 /* Exec format error */
#define EBADF           0x09 /* Bad file descriptor */
#define ECHILD          0x0a /* No child processes */
#define EAGAIN          0x0b /* Resource temporarily unavailable */
#define EWOULDBLOCK     0x0b /* Same value as EAGAIN */
#define ENOMEM          0x0c /* Cannot allocate memory */
#define EACCES          0x0d /* Permission denied */
#define EFAULT          0x0e /* Bad address */
#define ENOTBLK         0x0f /* Block device required */
#define EBUSY           0x10 /* Device or resource busy */
#define EEXIST          0x11 /* File exists */
#define EXDEV           0x12 /* Invalid cross-device link */
#define ENODEV          0x13 /* No such device */
#define ENOTDIR         0x14 /* Not a directory */
#define EISDIR          0x15 /* Is a directory */
#define EINVAL          0x16 /* Invalid argument */
#define ENFILE          0x17 /* Too many open files in system */
#define EMFILE          0x18 /* Too many open files */
#define ENOTTY          0x19 /* Inappropriate ioctl for device */
#define ETXTBSY         0x1a /* Text file busy */
#define EFBIG           0x1b /* File too large */
#define ENOSPC          0x1c /* No space left on device */
#define ESPIPE          0x1d /* Illegal seek */
#define EROFS           0x1e /* Read-only file system */
#define EMLINK          0x1f /* Too many links */
#define EPIPE           0x20 /* Broken pipe */
#define EDOM            0x21 /* Numerical argument out of domain */
#define ERANGE          0x22 /* Numerical result out of range */
#define EDEADLK         0x23 /* Resource deadlock avoided */
#define EDEADLOCK       0x23 /* Same value as EDEADLK */
#define ENAMETOOLONG    0x24 /* File name too long */
#define ENOLCK          0x25 /* No locks available */
#define ENOSYS          0x26 /* Function not implemented */
#define ENOTEMPTY       0x27 /* Directory not empty */
#define ELOOP           0x28 /* Too many levels of symbolic links */
#define ENOMSG          0x2a /* No message of desired type */
#define EIDRM           0x2b /* Identifier removed */
#define ECHRNG          0x2c /* Channel number out of range */
#define EL2NSYNC        0x2d /* Level 2 not synchronized */
#define EL3HLT          0x2e /* Level 3 halted */
#define EL3RST          0x2f /* Level 3 reset */
#define ELNRNG          0x30 /* Link number out of range */
#define EUNATCH         0x31 /* Protocol driver not attached */
#define ENOCSI          0x32 /* No CSI structure available */
#define EL2HLT          0x33 /* Level 2 halted */
#define EBADE           0x34 /* Invalid exchange */
#define EBADR           0x35 /* Invalid request descriptor */
#define EXFULL          0x36 /* Exchange full */
#define ENOANO          0x37 /* No anode */
#define EBADRQC         0x38 /* Invalid request code */
#define EBADSLT         0x39 /* Invalid slot */
#define EBFONT          0x3b /* Bad font file format */
#define ENOSTR          0x3c /* Device not a stream */
#define ENODATA         0x3d /* No data available */
#define ETIME           0x3e /* Timer expired */
#define ENOSR           0x3f /* Out of streams resources */
#define ENONET          0x40 /* Machine is not on the network */
#define ENOPKG          0x41 /* Package not installed */
#define EREMOTE         0x42 /* Object is remote */
#define ENOLINK         0x43 /* Link has been severed */
#define EADV            0x44 /* Advertise error */
#define ESRMNT          0x45 /* Srmount error */
#define ECOMM           0x46 /* Communication error on send */
#define EPROTO          0x47 /* Protocol error */
#define EMULTIHOP       0x48 /* Multihop attempted */
#define EDOTDOT         0x49 /* RFS specific error */
#define EBADMSG         0x4a /* Bad message */
#define EOVERFLOW       0x4b /* Value too large for defined data type */
#define ENOTUNIQ        0x4c /* Name not unique on network */
#define EBADFD          0x4d /* File descriptor in bad state */
#define EREMCHG         0x4e /* Remote address changed */
#define ELIBACC         0x4f /* Cannot access a needed shared library */
#define ELIBBAD         0x50 /* Accessing a corrupted shared library */
#define ELIBSCN         0x51 /* .lib section in a.out corrupted */
#define ELIBMAX         0x52 /* Too many shared libraries */
#define ELIBEXEC        0x53 /* Cannot exec a shared library directly */
#define EILSEQ          0x54 /* Invalid or incomplete multibyte character */
#define ERESTART        0x55 /* Interrupted syscall should be restarted */
#define ESTRPIPE        0x56 /* Streams pipe error */
#define EUSERS          0x57 /* Too many users */
#define ENOTSOCK        0x58 /* Socket operation on non-socket */
#define EDESTADDRREQ    0x59 /* Destination address required */
#define EMSGSIZE        0x5a /* Message too long */
#define EPROTOTYPE      0x5b /* Protocol wrong type for socket */
#define ENOPROTOOPT     0x5c /* Protocol not available */
#define EPROTONOSUPPORT 0x5d /* Protocol not supported */
#define ESOCKTNOSUPPORT 0x5e /* Socket type not supported */
#define EOPNOTSUPP      0x5f /* Operation not supported */
#define ENOTSUP         0x5f /* Same value as EOPNOTSUPP */
#define EPFNOSUPPORT    0x60 /* Protocol family not supported */
#define EAFNOSUPPORT    0x61 /* Address family not supported */
#define EADDRINUSE      0x62 /* Address already in use */
#define EADDRNOTAVAIL   0x63 /* Cannot assign requested address */
#define ENETDOWN        0x64 /* Network is down */
#define ENETUNREACH     0x65 /* Network is unreachable */
#define ENETRESET       0x66 /* Network dropped connection */
#define ECONNABORTED    0x67 /* Connection aborted */
#define ECONNRESET      0x68 /* Connection reset by peer */
#define ENOBUFS         0x69 /* No buffer space available */
#define EISCONN         0x6a /* Transport endpoint already connected */
#define ENOTCONN        0x6b /* Transport endpoint not connected */
#define ESHUTDOWN       0x6c /* Cannot send after shutdown */
#define ETOOMANYREFS    0x6d /* Too many references */
#define ETIMEDOUT       0x6e /* Connection timed out */
#define ECONNREFUSED    0x6f /* Connection refused */
#define EHOSTDOWN       0x70 /* Host is down */
#define EHOSTUNREACH    0x71 /* No route to host */
#define EALREADY        0x72 /* Operation already in progress */
#define EINPROGRESS     0x73 /* Operation now in progress */
#define ESTALE          0x74 /* Stale file handle */
#define EUCLEAN         0x75 /* Structure needs cleaning */
#define ENOTNAM         0x76 /* Not a XENIX named type file */
#define ENAVAIL         0x77 /* No XENIX semaphores available */
#define EISNAM          0x78 /* Is a named type file */
#define EREMOTEIO       0x79 /* Remote I/O error */
#define EDQUOT          0x7a /* Disk quota exceeded */
#define ENOMEDIUM       0x7b /* No medium found */
#define EMEDIUMTYPE     0x7c /* Wrong medium type */
#define ECANCELED       0x7d /* Operation canceled */
#define ENOKEY          0x7e /* Required key not available */
#define EKEYEXPIRED     0x7f /* Key has expired */
#define EKEYREVOKED     0x80 /* Key has been revoked */
#define EKEYREJECTED    0x81 /* Key was rejected */
#define EOWNERDEAD      0x82 /* Owner died */
#define ENOTRECOVERABLE 0x83 /* State not recoverable */
#define ERFKILL         0x84 /* RF-kill switch active */
#define EHWPOISON       0x85 /* Hardware memory error */

DLC_END_DECLS
