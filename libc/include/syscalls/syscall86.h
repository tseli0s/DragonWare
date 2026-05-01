/**********************************************************************
 * FILE: syscall86.h
 * PURPOSE: x86 (32 bit) system call routines, implemented in Assembly
 * PROJECT: DragonWare C Library
 * DATE: 12-2025
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 ***********************************************************************/

#pragma once

#include "stdint.h"

extern void __make_syscall_ia32_0param(int syscall);
extern void __make_syscall_ia32_1param(int syscall, uint32_t __param);
extern void __make_syscall_ia32_2param(int syscall, uint32_t __param1, uint32_t __param2);
extern void __make_syscall_ia32_3param(int syscall, uint32_t __param1, uint32_t __param2,
                                       uint32_t __param3);
extern void __make_syscall_ia32_4param(int syscall, uint32_t __param1, uint32_t __param2,
                                       uint32_t __param3, uint32_t __param4);
extern void __make_syscall_ia32_5param(int syscall, uint32_t __param1, uint32_t __param2,
                                       uint32_t __param3, uint32_t __param4, uint32_t __param5);

extern uint32_t __make_syscall_ia32_0param_reti32(int syscall);
extern uint32_t __make_syscall_ia32_1param_reti32(int syscall, uint32_t __param);
extern uint32_t __make_syscall_ia32_2param_reti32(int syscall, uint32_t __param1,
                                                  uint32_t __param2);
extern uint32_t __make_syscall_ia32_3param_reti32(int syscall, uint32_t __param1, uint32_t __param2,
                                                  uint32_t __param3);
extern uint32_t __make_syscall_ia32_4param_reti32(int syscall, uint32_t __param1, uint32_t __param2,
                                                  uint32_t __param3, uint32_t __param4);
extern uint32_t __make_syscall_ia32_5param_reti32(int syscall, uint32_t __param1, uint32_t __param2,
                                                  uint32_t __param3, uint32_t __param4,
                                                  uint32_t __param5);
