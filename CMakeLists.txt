cmake_minimum_required(VERSION 3.19.0)
project(DragonWare NONE)

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_C_STANDARD 23)

option(LEGACY_SUPPORT "Include support for legacy processors (older than Pentium, 486 mostly). Will produce slower code." OFF)
option(NXFPU_SUPPORT "Enable support for FPUs that are located outside the chip. Currently noop" OFF)
option(USE_CLANG_COMPILER "Use the clang compiler/LLVM toolchain to build DragonWare. Experimental." OFF)

if (USE_CLANG_COMPILER OR LEGACY_SUPPORT)
	include(cmake/ClangToolchain.cmake)
else()
	include(cmake/i686Toolchain.cmake)
endif()

if (USE_CLANG_COMPILER)
	include(cmake/ClangToolchain.cmake)
else()
	if (LEGACY_SUPPORT)
		include(cmake/i386Toolchain.cmake)
	else()
		include(cmake/i686Toolchain.cmake)
	endif()
endif()

include(cmake/OSVersion.cmake)
include(cmake/CompilerOverrides.cmake)

add_compile_definitions(__DRAGONWARE__)
if (NOT DEFINED CMAKE_BUILD_TYPE)
	message(STATUS "-- Note: CMAKE_BUILD_TYPE was not set in the options, setting it to debug.")
	set(CMAKE_BUILD_TYPE "Debug")
endif()
message("-- DragonWare Build Type: ${CMAKE_BUILD_TYPE}")

if (NOT(CMAKE_BUILD_TYPE STREQUAL "Release"))
	# Enables some extra assertions and checks
	add_compile_definitions(DRAGONWARE_DEBUG_MODE=1)
endif()
	

if (LEGACY_SUPPORT)
	add_compile_definitions(_DW_LEGACY_SUPPORT=1)
endif()

if (NXFPU_SUPPORT)
	if (NOT LEGACY_SUPPORT)
		message(WARNING "You have enabled the dedicated FPU chip support in DragonWare, but didn't enable legacy CPU support. Are you sure you want this? Because this makes no sense.")
	endif()
	add_definitions(_DW_NXFPU_SUPPORT=1)
endif()

enable_language(ASM_NASM)
include_directories(
			${CMAKE_SOURCE_DIR}/freestanding/common ${CMAKE_SOURCE_DIR}/freestanding/mm
			${CMAKE_SOURCE_DIR}/freestanding/string ${CMAKE_SOURCE_DIR}/freestanding/compat
			${CMAKE_SOURCE_DIR}/freestanding/arch
			${CMAKE_SOURCE_DIR}/dwuser/include
)

add_subdirectory(freestanding)
add_subdirectory(sys)
add_subdirectory(libc)
add_subdirectory(dwuser)
add_subdirectory(base) # That day has come. 14/03/2026.
add_subdirectory(bootmanager)

include(cmake/sysroot.cmake)

set(KERNEL_BINARY $<TARGET_FILE:kernel.sys>)
set(LICENSE_FILE ${CMAKE_SOURCE_DIR}/COPYING)

# These are generated in base/CMakeLists.txt
get_property(USER_BINARIES_LIST GLOBAL PROPERTY DRAGONWARE_USER_BINARIES)
get_property(USER_TARGET_LIST GLOBAL PROPERTY DRAGONWARE_USER_TARGETS)

add_custom_target(sysroot DEPENDS kernel.sys
		  COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSROOT_PATH}
		  COMMAND ${CMAKE_COMMAND} -E make_directory ${SYSTEM_PATH}
		  COMMAND ${CMAKE_COMMAND} -E make_directory ${BOOTFILES_PATH}
		  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROGRAMS_PATH}
		  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BINARY} ${SYSTEM_PATH}/kernel.sys
		  COMMAND ${CMAKE_COMMAND} -E copy ${USER_BINARIES_LIST} ${PROGRAMS_PATH}
		  # FIXME: Of course programs shouldn't be in the root directory!
		  # That's just a temporary setup for the DragonWare Boot Manager which happens to be
		  # too dumb to read from subdirectories.
		  COMMAND ${CMAKE_COMMAND} -E copy ${USER_BINARIES_LIST} ${SYSROOT_PATH}
		  COMMENT "Build sysroot file hierarchy for the disk image"
		  VERBATIM)

add_custom_target(bootcd DEPENDS sysroot BootImages
		  # Temporary thing, until the bootloader can read subdirectories. Which I'm too lazy to implement for now.
		  COMMAND ${CMAKE_COMMAND} -E copy ${KERNEL_BINARY} ${SYSROOT_PATH}/KRNLIA32.SYS
		  COMMAND ${CMAKE_COMMAND} -E cat ${BOOTIMG_CD} $<TARGET_FILE:core.img> > ${SYSROOT_PATH}/BOOTMGR.SYS
		  COMMAND xorriso -as mkisofs -R -r -J -b /BOOTMGR.SYS -no-emul-boot -boot-load-size 4 -boot-info-table -o ${BOOTCD} ${SYSROOT_PATH}
		  COMMENT "Create Boot CD for DragonWare using DragonWare Boot Manager as the bootloader"
		  VERBATIM)

add_custom_target(everything DEPENDS dlibc sysroot bootcd
		  COMMENT "Build everything (Disk image under ${CMAKE_SOURCE_DIR}/bootcd.iso)"
		  # To work well with the scripts under tools/
		  COMMAND ${CMAKE_COMMAND} -E copy ${BOOTCD} ${CMAKE_SOURCE_DIR}
		  VERBATIM)
