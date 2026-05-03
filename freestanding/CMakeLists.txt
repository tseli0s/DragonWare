project(FreestandingLibrary)

set(FREESTANDING_SOURCES_C    ${CMAKE_CURRENT_SOURCE_DIR}/mm/mmutils.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/string/string.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/compat/arith64.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/compat/atomic.c
)
set(FREESTANDING_SOURCES_ASM  ${CMAKE_CURRENT_SOURCE_DIR}/mm/mmutils.asm
                              ${CMAKE_CURRENT_SOURCE_DIR}/string/string.asm
                              ${CMAKE_CURRENT_SOURCE_DIR}/arch/power.asm)

set(DWFSL_COMPILE_OPTS -ffreestanding -nostdlib -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
                        -Wuninitialized -fno-lto -fno-omit-frame-pointer -fno-builtin -m32
                        -Os -fvisibility=hidden -fno-strict-aliasing -fno-pic
                        -fno-pie -mno-sse -mno-sse2 -mno-mmx)

add_library(freestanding86.sys OBJECT ${FREESTANDING_SOURCES_ASM})
target_compile_options(freestanding86.sys PRIVATE -felf32)

add_library(dwfsl STATIC ${FREESTANDING_SOURCES_C} $<TARGET_OBJECTS:freestanding86.sys>)
target_compile_options(dwfsl PRIVATE ${DWFSL_COMPILE_OPTS})
target_compile_definitions(dwfsl PRIVATE _FREE_STANDING_DWFSL=1)
target_include_directories(dwfsl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/arch
                                         ${CMAKE_CURRENT_SOURCE_DIR}/common
                                         ${CMAKE_CURRENT_SOURCE_DIR}/compat
                                         ${CMAKE_CURRENT_SOURCE_DIR}/mm
                                         ${CMAKE_CURRENT_SOURCE_DIR}/string)
