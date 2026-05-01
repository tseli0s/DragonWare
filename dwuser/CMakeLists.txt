project(DragonWareUserLibrary)
set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
include_directories(${CMAKE_SOURCE_DIR}/libc/include)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(DW_USER_SRC syscall/syscall.c api/object.c api/message.c)
add_library(dwuser STATIC ${DW_USER_SRC})
target_compile_options(
        dwuser
        PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
                -Wall -Wextra -Wuninitialized -Wpedantic -nostdinc -nostdlib
                -fno-pic -fno-pie -mno-sse -mno-mmx -mno-avx -mno-sse2
        >)
target_link_options(dlibc PRIVATE -ffreestanding -nostdlib)
