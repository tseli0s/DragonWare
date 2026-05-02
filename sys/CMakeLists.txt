project(DragonWareKernel)
if (NOT (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang"))
        message(FATAL_ERROR "DragonWare requires GCC or clang to be built. Other compilers may not work.")
endif()

set(C_SOURCES "")
set(ASM_SOURCES "")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if (${ARCH} STREQUAL "IA32")
     set(DDK_SYSDIR ${CMAKE_CURRENT_SOURCE_DIR}/ddk/ia32)
     list(APPEND C_SOURCES
          ${DDK_SYSDIR}/gdt.c
          ${DDK_SYSDIR}/idt.c
          ${DDK_SYSDIR}/irq.c
          ${DDK_SYSDIR}/pic.c
          ${DDK_SYSDIR}/cpu.c
          ${DDK_SYSDIR}/vmm.c
          ${DDK_SYSDIR}/ctxswitch.c
          ${DDK_SYSDIR}/smbios.c
          ${DDK_SYSDIR}/exception.c
          )

     list(APPEND ASM_SOURCES
          ${DDK_SYSDIR}/gdt_x86.asm
          ${DDK_SYSDIR}/ctxswitch.asm
          ${DDK_SYSDIR}/sysenter.asm
          ${DDK_SYSDIR}/isr.asm
          ${DDK_SYSDIR}/vmm_x86.asm
          ${DDK_SYSDIR}/kcpuid_x86.asm
          ${DDK_SYSDIR}/boot/bootia32.asm
          ${DDK_SYSDIR}/boot/multiboot.asm)
else()
     message(FATAL_ERROR "DragonWare's DDK is only ported to IA32 for now. Please build DragonWare for a 32 bit x86 target, with an appropriate compiler")
endif()

list(APPEND C_SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/drivers/initdriver.c
     ${CMAKE_CURRENT_SOURCE_DIR}/fonts/glyph.c
     ${CMAKE_CURRENT_SOURCE_DIR}/fonts/psfparse.c
     ${CMAKE_CURRENT_SOURCE_DIR}/fonts/spleen.c
     ${CMAKE_CURRENT_SOURCE_DIR}/init/main.c
     ${CMAKE_CURRENT_SOURCE_DIR}/init/bootinfo.c
     ${CMAKE_CURRENT_SOURCE_DIR}/init/wfi.c
     ${CMAKE_CURRENT_SOURCE_DIR}/init/state.c
     ${CMAKE_CURRENT_SOURCE_DIR}/mem/frame.c
     ${CMAKE_CURRENT_SOURCE_DIR}/mem/mm.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/log.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/stdio.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/kmalloc.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/early_kmalloc.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/panic.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/stackprot.c
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/sanitize.c
     ${CMAKE_CURRENT_SOURCE_DIR}/syscall/syscall.c
     ${CMAKE_CURRENT_SOURCE_DIR}/syscall/ipc.c
     ${CMAKE_CURRENT_SOURCE_DIR}/syscall/object.c
     ${CMAKE_CURRENT_SOURCE_DIR}/syscall/usercopy.c
     ${CMAKE_CURRENT_SOURCE_DIR}/time/systime.c
     ${CMAKE_CURRENT_SOURCE_DIR}/time/timer.c
     ${CMAKE_CURRENT_SOURCE_DIR}/video/output.c
     ${CMAKE_CURRENT_SOURCE_DIR}/iomgr/devmgr.c
     ${CMAKE_CURRENT_SOURCE_DIR}/iomgr/port.c
     ${CMAKE_CURRENT_SOURCE_DIR}/iomgr/section.c
     ${CMAKE_CURRENT_SOURCE_DIR}/iomgr/object.c
     ${CMAKE_CURRENT_SOURCE_DIR}/iomgr/node.c
     ${CMAKE_CURRENT_SOURCE_DIR}/task/task.c
     ${CMAKE_CURRENT_SOURCE_DIR}/task/process.c
     ${CMAKE_CURRENT_SOURCE_DIR}/task/message.c
     ${CMAKE_CURRENT_SOURCE_DIR}/sched/schedule.c
)

if (ARCH STREQUAL IA32)
     list(APPEND ASM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/task/task.asm)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
     # Performance speed up - Writing to the serial takes some time depending on
     # the computer model and whether a serial port is even present.
     add_compile_definitions(__K_DISABLE_SERIAL_OUTPUT=1)
else()
     list(APPEND KERNEL_COMPILE_OPTS -fsanitize=undefined)
endif()

set(BUILDID_FILE ${CMAKE_CURRENT_SOURCE_DIR}/BUILDID)
if (EXISTS ${BUILDID_FILE})
	file(READ ${BUILDID_FILE} BUILDID)
	string(STRIP "${BUILDID}" BUILDID) # Just to be sure
	math(EXPR COUNTER "${BUILDID} + 1")
else()
	set(BUILDID 0)
	message(WARNING "BUILDID not set (File missing), setting it to 0.")
endif()
file(WRITE "${BUILDID_FILE}" "${COUNTER}")

enable_language(C)
add_library(kernel86.sys OBJECT ${ASM_SOURCES})
target_compile_options(kernel86.sys PRIVATE -felf32)

add_subdirectory(drivers) # To provide drvobj.sys, which is a collection of object files to put in the drivers section.

add_executable(kernel.sys ${C_SOURCES} $<TARGET_OBJECTS:kernel86.sys> $<TARGET_OBJECTS:drvobj.sys>)

set(KERNEL_COMPILE_OPTS -ffreestanding -nostdlib -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
                        -Wuninitialized -fno-lto -fno-omit-frame-pointer -fno-builtin -m32
                        -fvisibility=hidden -fno-strict-aliasing -fstack-protector-strong -fno-pic
                        -fno-pie -mno-sse -mno-mmx -mno-avx -mno-sse2)

target_compile_options(kernel.sys PRIVATE ${KERNEL_COMPILE_OPTS})
target_link_libraries(kernel.sys PRIVATE gcc dwfsl)
target_link_options(kernel.sys PRIVATE -T "${CMAKE_CURRENT_SOURCE_DIR}/linker.lds" -ffreestanding -nostdlib)
get_target_property(KERNEL_CFLAGS_LIST kernel.sys COMPILE_OPTIONS)
string(JOIN " " KERNEL_CFLAGS ${KERNEL_CFLAGS_LIST})
target_compile_definitions(kernel.sys PRIVATE __DRAGONWARE_SYS__ __KERNEL_BUILDID__=${BUILDID} _KERNEL_CFLAGS=\"${KERNEL_CFLAGS}\")
