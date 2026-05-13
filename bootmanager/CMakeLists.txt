project(BootManager)

set(BOOTIMG_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/bootsect/hdd/boot.asm)
set(BOOTCDIMG_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/bootsect/cdrom/bootcd.asm)

set(CORE_SOURCES core/init/main.c core/init/loader.asm core/cpu/idt86.c core/cpu/irq.c
                 core/cpu/isrgen.asm core/cpu/irqgen.asm core/cpu/pic.c core/common/highmem.c
                 core/kbd/kbd.c core/timer/pit.c core/textmode/vgatext.c core/textmode/tui.c
                 core/init/bootentry.c core/storage/ata.c core/common/memdetect.c
                 core/storage/partition.c core/storage/mbr.c core/init/jmpkrnl.asm
                 core/common/error.c core/fs/fs.c core/fs/fat32/fat32.c core/fs/iso9660/iso9660.c
                 core/common/frame.c core/common/kmalloc.c core/textmode/dbgprint.c
                 core/init/mbutils.c core/elfldr/elfloader.c core/cpu/modeset.asm)

set(BOOTIMG ${CMAKE_CURRENT_BINARY_DIR}/boot.img        PARENT_SCOPE)
set(BOOTIMG_CD ${CMAKE_CURRENT_BINARY_DIR}/bootcd.img   PARENT_SCOPE)

# Second stage. It doesn't matter in what order we build the stages, as long as they're all present
# when we create the disk image.
add_executable(core.img ${CORE_SOURCES})
set_target_properties(core.img PROPERTIES LINK_FLAGS "-nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/core/linker.lds")
target_compile_options(core.img PRIVATE $<$<COMPILE_LANGUAGE:ASM>:-felf32>)
target_compile_options(core.img PRIVATE
                        $<$<COMPILE_LANGUAGE:C>:-ffreestanding -nostdlib -fno-builtin -Wall -mno-sse -mno-avx -mno-mmx
                        -mno-sse2 -Wextra -Wshadow -Wwrite-strings -Oz -g0 -fno-strict-aliasing -fno-lto>
                        )

target_include_directories(core.img PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/core/ ${CMAKE_CURRENT_SOURCE_DIR}/core/common)
target_include_directories(core.img PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib
                                            ${CMAKE_SOURCE_DIR}/freestanding/common
                                            ${CMAKE_SOURCE_DIR}/freestanding/mm
                                            ${CMAKE_SOURCE_DIR}/freestanding/string
                                            ${CMAKE_SOURCE_DIR}/freestanding/compat
                                            ${CMAKE_SOURCE_DIR}/freestanding/arch/ia32)

target_link_libraries(core.img PRIVATE dwfsl)
add_custom_command(TARGET core.img POST_BUILD
                   COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/align_to_sector.py $<TARGET_FILE:core.img>
                   COMMENT "Pad core.img to expected sector count by default"
                   VERBATIM)

set(BOOTIMG ${CMAKE_CURRENT_BINARY_DIR}/boot.img)
set(BOOTIMG_CD ${CMAKE_CURRENT_BINARY_DIR}/bootcd.img)

add_custom_command(OUTPUT "${BOOTIMG_CD}"
                   COMMAND ${CMAKE_ASM_NASM_COMPILER} -fbin ${BOOTCDIMG_SRC} -I ${CMAKE_CURRENT_SOURCE_DIR}/bootsect -o "${BOOTIMG_CD}"
                   DEPENDS "${BOOTCDIMG_SRC}"
                   COMMENT "Build no emulation El Torito boot sector"
                   VERBATIM)
add_custom_target(generate_bootcd_img DEPENDS "${BOOTIMG_CD}")

add_custom_command(OUTPUT "${BOOTIMG}"
                   COMMAND ${CMAKE_ASM_NASM_COMPILER} -fbin ${BOOTIMG_SRC} -I ${CMAKE_CURRENT_SOURCE_DIR}/bootsect -o "${BOOTIMG}"
                   DEPENDS "${BOOTIMG_SRC}"
                   COMMENT "Build boot sector code image"
                   VERBATIM)

add_custom_target(generate_boot_img DEPENDS "${BOOTIMG}")
add_custom_target(BootImages ALL COMMENT "Build all boot images" DEPENDS core.img generate_boot_img generate_bootcd_img)
