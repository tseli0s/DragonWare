project(DWUserland)
include_directories(${CMAKE_SOURCE_DIR}/libc/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/dwuser/include)

# Prevent us from using any hosted C headers, even those that are considered "harmless"
# like stdint.h
set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")

macro(add_user_program binname)
        add_executable(${binname}.run ${ARGN})
        target_link_libraries(${binname}.run dwuser dlibc)
        target_link_options(
                ${binname}.run PRIVATE
                "-T${CMAKE_CURRENT_SOURCE_DIR}/linker-flat.lds"
                "-ffreestanding" "-nostdlib"
        )

        # Note how we still build the binaries as freestanding, to avoid linking errors because the compiler tries to
        # use the host (Possibly Linux 64-bit) crt0 files and libc.
        target_compile_options(
                ${binname}.run
                PRIVATE
                -ffreestanding -nostdlib -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
                -Wuninitialized -fno-lto -fno-omit-frame-pointer -fno-builtin -m32 -fvisibility=hidden
                -fno-strict-aliasing -fno-pic -fno-pie -mno-sse -mno-mmx -mno-avx -mno-sse2
        )
        target_include_directories(${binname}.run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_include_directories(${binname}.run PRIVATE ${CMAKE_SOURCE_DIR}/dwuser/include)
        set_property(GLOBAL APPEND PROPERTY DRAGONWARE_USER_BINARIES $<TARGET_FILE:${binname}.run>)
        set_property(GLOBAL APPEND PROPERTY DRAGONWARE_USER_TARGETS ${binname}.run)
endmacro()

add_user_program(fbsrv fbsrv/main.c)
add_user_program(ps2kbd ps2kbd/ps2kbd.c)
add_user_program(vgacons vgacons/console.c vgacons/driver.c)
add_user_program(dcp dcp/main.c dcp/command.c)
