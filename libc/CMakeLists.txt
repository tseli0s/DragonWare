project(dlibc)

set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
set(GENERIC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/generic/stdlib.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/generic/string.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/generic/stdio.c
                    ${CMAKE_CURRENT_SOURCE_DIR}/generic/errno.c)
set(COMPAT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/compat/arith64.c)
set(INCLUDEDIR      ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (${ARCH} STREQUAL "IA32")
        list(APPEND ASM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/syscalls/syscall86.asm ${CMAKE_CURRENT_SOURCE_DIR}/crt0.asm)
endif()

add_library(dlibc STATIC ${GENERIC_SOURCES} ${SYSCALL_SOURCES} ${COMPAT_SOURCES} ${ASM_SRC})
target_include_directories(dlibc PRIVATE ${INCLUDEDIR})
target_compile_options(
        dlibc
        PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
                -ffreestanding -nostdlib -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
                -Wuninitialized -fno-lto -fno-omit-frame-pointer -fno-builtin -m32
                -fvisibility=hidden -fno-strict-aliasing -fno-pic -fno-pie -mno-sse -mno-mmx -mno-avx -mno-sse2
        >)
target_link_options(dlibc PRIVATE -ffreestanding -nostdlib)

