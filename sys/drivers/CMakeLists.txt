set(VBE86_SRC   ${CMAKE_CURRENT_SOURCE_DIR}/vbe86/driver.c)
set(SER86_SRC   ${CMAKE_CURRENT_SOURCE_DIR}/serial86/com.c)
set(VGATEXT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/vgatext/textmode.c)

set(ALL_DRIVERS_SOURCE ${PS2KBD_SRC} ${VBE86_SRC} ${SER86_SRC} ${VGATEXT_SRC})
add_library(drvobj.sys OBJECT ${ALL_DRIVERS_SOURCE})
target_compile_options(drvobj.sys PRIVATE -ffreestanding -nostdlib -Wall -Wextra -Wpedantic -Wno-error=pedantic -Wshadow -Wconversion
	                -Wsign-conversion -Wuninitialized -fno-lto -fno-omit-frame-pointer -fno-builtin -m32)
