/**********************************************************************
 * FILE: port.c
 * PURPOSE: Port object wrappers
 * PROJECT: DragonWare User Library
 * DATE: 09-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <kernelapi.h>
#include <message.h>
#include <object.h>

[[nodiscard(
        "Returned handle is a handle that references the port object and must not be discarded.")]]
Handle CreatePort(const char *name) {
        Handle port = CreateObject(NullPointer, OBJ_PORT, 0);
        if (port >= 0) {
                if (InvokeObject(port, PORT_CREATE, (void *)name) != STATUS_OK)
                        goto bad;
                else
                        return port;
        } else
                goto bad;
bad:
        DeleteObject(port);
        return -1;
}

Handle OpenPort(const char *name) {
        Handle port = CreateObject(NullPointer, OBJ_PORT, 0);
        if (port < 0) goto bad;

        if (InvokeObject(port, PORT_OPEN, (void*)name) != STATUS_OK) goto bad;
        return port;
bad:
        if (port >= 0) DeleteObject(port);
        return -1;
}


Status IPCCall(Message *msgbuf, Handle send, Handle recv) {
        Status s1 = SendMessage(send, msgbuf, SIZE_OF_MESSAGE(*msgbuf));
        if (s1 != STATUS_OK) return s1;

        return ReceiveMessage(recv, msgbuf);
}
