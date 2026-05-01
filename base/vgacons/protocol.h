/**********************************************************************
 * FILE: driver.h
 * PURPOSE: VGA text mode driver utilities
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

/* VGA console protocol version 0 */
#define VGACONS_PROTOCOL_V0             ((u16)0xAF82)

/*
 * Message type: Request character draw (automatic coordinates)
 *
 * Payload contents:
 * - Byte 0 = Character to draw. Must be an ASCII printable character.
 * - Bytes 1-3 = Reserved, must be zeroed out
 */
#define VGACONS_REQUEST_CHAR_DRAW       (0xB8)

/*
 * Message type: Request character draw (specific coordinates)
 *
 * Payload contents:
 * - Byte 0 = Character to draw. Must be an ASCII printable character.
 * - Byte 1 = Column (X axis, must be between 0 and VGA_HEIGHT (25))
 * - Byte 2 = Row (Y axis, must be between 0 and VGA_WIDTH (80))
 * - Byte 3 = Reserved, must be zeroed out
 */
#define VGACONS_REQUEST_CHAR_DRAW_FIXED (0xB2)

/*
 * Message type: Request string draw (automatic coordinates)
 *
 * Payload contents: Must contain a null-terminated ASCII string to print. The server will refuse to
 * print more than 256 bytes at once.
 */
#define VGACONS_REQUEST_STRING_DRAW     (0xE1)

/*
 * Message type: Claim console events for the application.
 *
 * Reply: STATUS_OK if the process now receives all the console events. STATUS_BAD if another
 * process is currently claiming the console.
 */
#define VGACONS_CLAIM_CONSOLE           (0xA4)
