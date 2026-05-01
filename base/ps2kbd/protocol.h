/**********************************************************************
 * FILE: protocol.h
 * PURPOSE: DragonWare Keyboard Server protocol exports
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

/* Protocol version 0 (Other protocols may be invented in the future) */
#define KBD_PROTOCOL_V0       ((u16)(0x5E74))

/*
 * Message type: Key dispatch (The server received a keyboard event and dispatches it to the active
 * listener).
 *
 * Payload contents:
 * - Byte 0 = Key pressed (As ASCII character)
 * - Byte 1 = 1 if key release, 0 otherwise.
 * - Bytes 2-3 = Set to 0 automatically.
 */
#define KBD_KEY_DISPATCH      (0x01)

/*
 * Message type: Modifier dispatch (The server received a keyboard event, but that key is
 * unprintable (Modifier or extended key))
 *
 * Payload contents:
 * - Byte 0: Modifier specifier. Currently unused.
 * - Byte 1: 1 if key release, 0 otherwise.
 * - Bytes 2-3: Set to 0 automatically.
 */
#define KBD_MODIFIER_DISPATCH (0x02)

/*
 * Message type: Request to listen to the keyboard server's events. (The server will dispatch
 * keyboard events to this process, if it is accepted).
 *
 * Reply expected: STATUS_OK if the calling process is selected by the server as keyboard.
 * STATUS_BAD if there is already an active listener or the server does not trust the calling
 * process. The reply is stored in the first byte of the payload.
 */
#define KBD_LISTENER_REQUEST  (0x03)
