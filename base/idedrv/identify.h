/**********************************************************************
 * FILE: identify.h
 * PURPOSE: IDE/ATA drive IDENTIFY command helpers
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kerneltypes.h>

/**
 * @brief Probes for a connected ATA/IDE compatible storage medium on the given (bus, slot)
 * configuration. Returns a status code depending on the result.
 * @param primary 0 for the primary bus, 1 for the secondary bus.
 * @param master 0 for the master drive, 1 for the slave drive.
 * @returns STATUS_OK if there's a drive connected in that (master, bus) configuration. STATUS_BAD
 * if a hardware error occured, STATUS_NOT_FOUND if a drive is not detected in that slot.
 * @since v0.0.2
 */
Status IdentifyDrive(int primary, int master);
