/**********************************************************************
 * FILE: main.c
 * PURPOSE: High level entry point of the DragonWare kernel
 * PROJECT: DragonWare Kernel
 * DATE: 08-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#include <mmutils.h>
#include <vendor/multiboot.h>

#include "assert.h"
#include "ddk/ia32/idt.h"
#include "iomgr/port.h"
#include "sched/schedule.h"
#include "task/process.h"
#include "task/task.h"
#include "time/timer.h"
#ifdef __i386__
#include "ddk/ia32/cpu.h"
#include "ddk/ia32/paging.h"
#include "ddk/ia32/smbios.h"
#include "ddk/ia32/vmm.h"
#endif /* __i386__ */
#include <kstring.h>
#include <ktypes.h>
#include <log.h>
#include <macros.h>
#include <panic.h>

#include "bootinfo.h"
#include "drivers/initdriver.h"
#include "iomgr/devmgr.h"
#include "mem/frame.h"
#include "mem/mm.h"
#include "time/systime.h"
#include "wfi.h"

[[noreturn]]
static inline void __idle(void) {
        while (true) {
                Thread* list = GetSchedulerThreadList();
                if (list) {
                        PushCritical();
                        Thread* iter = list;
                        while (iter) {
                                Thread* next_node = iter->next;
                                if (iter->state == THREAD_TERMINATED) {
                                        RemoveThreadFromScheduler(iter);
                                        DeleteThread(iter);
                                }
                                iter = next_node;
                        }
                        PopCritical();
                }
                __asm__ volatile("hlt");
        }
}

static void LogKernelBuildInformation(void) {
        LogMessage(LOG_INFO, "DragonWare Version %d.%d.%d-%s", DRAGONWARE_VERSION_MAJOR,
                   DRAGONWARE_VERSION_MINOR, DRAGONWARE_VERSION_PATCH, DRAGONWARE_VERSION_SUFFIX);
        LogMessage(LOG_INFO, "This build was compiled using %s at %s", __VERSION__, __TIMESTAMP__);
        LogMessage(LOG_INFO,
                   "DragonWare is distributed under the terms of the GNU General Public License, "
                   "version 3.");
}

void SystemKernelInit(uintptr_t multiboot_addr_phys) {
        volatile Multiboot* _bootinfo_phys = (Multiboot*)multiboot_addr_phys;
        RegisterBootInformation((Multiboot*)_bootinfo_phys);
        Multiboot* mbi = GetBootInformationStructure();

        if (!(mbi->flags & MULTIBOOT_MMAP))
                FatalError(
                        "Bootloader did not provide a memory "
                        "map! Kernel cannot "
                        "manage host memory, halting execution.");
        LogInit();
        ArchInit();

        LogKernelBuildInformation();
        RegisterBootModules();
        AddMemoryRegions(mbi);
        InitFrameManager();
        if (InitVirtualMemoryManager() != STATUS_OK)
                FatalError("Cannot bring up the virtual memory manager");

        StartSystemTimer();
        SystemTime boottime = GetSystemTime();
        LogMessage(LOG_INFO, "Boot time: %d/%d/%d %d:%d:%d", boottime.day, boottime.month,
                   boottime.year, boottime.hour, boottime.minutes, boottime.seconds);

        /* Has to be brought up as soon as possible. */
        if (InitDeviceManager() != STATUS_OK)
                FatalError("InitDeviceManager() did not return STATUS_OK");

        if (InitializePortTree() != STATUS_OK)
                FatalError("InitializePortTree() did not return STATUS_OK");

        SMBIOSData* smbios_data = NullPointer;
        /* WARNING: This is a lower memory pointer, don't access it directly. */
        uintptr_t   smbios_ptr  = FindSMBIOSDataArea();
        if (smbios_ptr) {
                smbios_data = SMBIOSDataFromPointer(smbios_ptr);
                DumpSMBIOSData(smbios_data);
        }

        if (BringBuiltinDriversOnline() != STATUS_OK)
                FatalError(
                        "A builtin driver was unable to be initialized. Your system may be "
                        "unsupported and DragonWare is stopping boot to ensure system safety.");

        /* The bootloader name is also written in low memory, we have to temporarily map it to read
         * it. The block here is for readability only, it can be removed safely. */
        {
                char*     bootid   = (char*)mbi->bootloader;
                uintptr_t virtaddr = (aligndown((uintptr_t)bootid, PAGE_SIZE));
                MapSinglePage(virtaddr, virtaddr, PAGE_PRESENT);
                LogMessage(LOG_INFO, "Bootloader ID: %s", (char*)mbi->bootloader);
                UnmapSinglePage(virtaddr);
        }

        if (mbi->flags & MULTIBOOT_INFO_BOOTDEV) {
                const u8 bootdisk = (u8)(mbi->boot_device & 0xFF);
                LogMessage(LOG_INFO, "Bootloader reports BIOS disk index 0x%x is the boot disk",
                           bootdisk);
        } else
                LogMessage(LOG_WARNING, "Bootloader did not specify BIOS boot media");

        FlushAllLogs();

        /* Create an idle thread for the kernel, so that the scheduler always has a task to run */
        Thread* idle_task = AllocateThread(__idle, NullPointer);
        AddThreadToScheduler(idle_task);

        if (!mbi->mods_count)
                FatalError(
                        "No modules provided by the bootloader. Please reboot your machine, and in "
                        "the boot menu, pass the path to the system servers that must be loaded.");
        else {
                /* We are now ready to load the servers. Notice how we grant them permission to
                 * access hardware directly, otherwise they won't work. */
                for (unsigned int i = 0; i < mbi->mods_count; i++) {
                        MultibootModule init;
                        Status          module_fetch_status = GetBootModuleAt(i, &init);
                        kassert(module_fetch_status == STATUS_OK);

                        uintptr_t start_phys = aligndown(init.start, PAGE_SIZE);
                        Size      n_pages = (alignup(init.end, PAGE_SIZE) - start_phys) / PAGE_SIZE;

                        MapMemoryRange(start_phys, start_phys, PAGE_PRESENT | PAGE_RW, n_pages);

                        Process* new_proc =
                                CreateProcess(0, (void*)init.start, init.end - init.start);
                        SetProcessCapabilities(new_proc,
                                               PROC_C_IOPL | PROC_C_SERVER | PROC_C_IRQ_DISPATCH);
                        UnmapMemoryRange(start_phys, n_pages);
                }
        }
        /* Now that we have a couple of threads loaded, load the scheduler before we enable
         * interrupts */
        InitSchedulerState();
        WaitForInterrupts();
}
