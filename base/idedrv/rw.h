/**********************************************************************
 * FILE: rw.h
 * PURPOSE: Read/Write I/O operations for ATA/IDE drives
 * PROJECT: DragonWare Base System
 * DATE: 08-2026
 * AUTHORS: Aggelos Tselios <aggelostselios777@gmail.com> (adapted from xv6:
 *https://github.com/mit-pdos/xv6-public) LICENSE: GPL-3.0-or-later
 *(https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include <kernelapi.h>
#include <kerneltypes.h>
#include <object.h>

#include "idedrv/protocol.h"

/**
 * @brief Read a single 512-byte sized sector into @p buf
 * @param irq_handle Handle to the IRQ dispatch port where the kernel will notify of IRQ14/15
 * arriving
 * @param irq_descr The @ref IRQBindingDescriptor associated with @p handle to ACK the IRQ
 * @param bus 0 for primary bus, 1 for secondary bus.
 * @param master 0 for master drive, 1 for slave drive.
 * @param lba LBA to read from. Must be a 28-bit integer. High four bits are ignored.
 * @param[out] buf Where to write the data. Must not be a @ref NullPointer.
 * @returns An appropriate @ref IDEDRVStatusReply code for the operation.
 */
[[gnu::nonnull]]
IDEDRVStatusReply ReadFromDisk(Handle irq_handle, IRQBindingDescriptor irq_descr, int bus,
                               int master, u32 lba, void *buf);

/**
 * @brief Write the provided 512 byte block into the LBA @p lba of the given drive @p master on bus
 * @p bus.
 * @param irq_handle Handle to the IRQ dispatch port where the kernel will notify of IRQ14/15
 * arriving.
 * @param irq_descr The @ref IRQBindingDescriptor associated with @p handle to ACK the IRQ
 * @param bus 0 for primary bus, 1 for secondary bus.
 * @param master 0 for master drive, 1 for slave drive.
 * @param lba LBA to write the data to. High four bits are ignored.
 * @param[in] buf The data to write to the disk. Must contain 512 bytes of data that will be written
 * into the LBA @p lba of the selected drive. (See note below)
 * @note If less than 512 bytes of data needs to be written into LBA @p lba (eg. Saving only 10
 * bytes of changes in a single file), currently you must read the LBA @p lba into memory first,
 * replace those 10 bytes and then write back the @p lba. This will be replaced in the future with a
 * better API.
 * @returns An appropriate @ref IDEDRVStatusReply code for the operation.
 */
[[gnu::nonnull]]
IDEDRVStatusReply WriteToDisk(Handle irq_handle, IRQBindingDescriptor irq_descr, int bus,
                              int master, u32 lba, void *buf);
