; ----------------------------------------------------------------------------------------
; FILE: booia32.asm
; PURPOSE: Kernel executable entry point - Saves multiboot state, maps the kernel to the higher half, and prepares the C environment.
; PROJECT: DragonWare Kernel
; DATE: 08-2025
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

[bits 32]

%include "lib/asm86/macros.asm"

extern _sbss
extern FatalError

%define KERNEL_VMA 0xC0000000
%define KERNEL_LMA 0x00100000

%define PAGE_PRESENT 	0x1
%define PAGE_RW		0x2
%define PAGE_SIZE 	0x1000
%define STACK_SIZE	0x4000 ; 4 pages times 4KBs each, that's 16KBs of bootstrap stack memory
%define VIRT_OFFSET 	(KERNEL_VMA - KERNEL_LMA)
%define KERNEL_PDE	(KERNEL_VMA >> 22)

%define PAGE_TABLE_SIZE_DWORDS          1024
%define PAGE_DIRECTORY_SIZE_DWORDS      1024
%define N_REGISTERS                     2

extern SystemKernelInit
extern default_state

section .bss
align 16
BootRegisters: 	resd N_REGISTERS
stack:
	resb STACK_SIZE

section .data
align PAGE_SIZE

PageDirectory: 	times PAGE_DIRECTORY_SIZE_DWORDS        dd 0x0
BootPageTable:	times PAGE_TABLE_SIZE_DWORDS            dd 0x0
MMIOPageTable:  times PAGE_TABLE_SIZE_DWORDS            dd 0x0
KernelReturned: db "Kernel returned from execution without a valid path to do so!", 0

section .text
global _SystemBootstrapRoutine

_SystemBootstrapRoutine:
        cli
        mov     [BootRegisters - VIRT_OFFSET],      eax
        mov     [BootRegisters - VIRT_OFFSET + 4],  ebx
    
        xor     ecx, ecx
        inc     ecx

        ; The kernel expects the lower half to be identity mapped for the early bootstrap process, so that
        ; it can access some information in low memory. That mapping is later removed to make space for userspace processes.
.IdentityMapLowerHalf:
        mov     eax, ecx
        shl     eax, 12
        or      eax, PAGE_PRESENT | PAGE_RW
    
        mov     [MMIOPageTable - VIRT_OFFSET + ecx * 4], eax
    
        ; Because of the kernel architecture, we will identity map the entire first 4MBs of memory until
        ; InitVirtualMemoryManager removes all unused mappings. This is because, in early boot, we assume AllocateFrame()
        ; returns a page that was already mapped here, so that we don't need to map it manually and complicate the logic.
        inc     ecx
        cmp     ecx, PAGE_TABLE_SIZE_DWORDS-1
        jne     .IdentityMapLowerHalf

        mov     eax, MMIOPageTable - VIRT_OFFSET
        or      eax, PAGE_PRESENT | PAGE_RW

        mov     [PageDirectory - VIRT_OFFSET], eax

        xor     ecx, ecx
; Now map the kernel to the higher half. Only a single megabyte is mapped - Extra memory is taken care
; of in InitVirtualMemoryManager, if needed.
.MapKernelToHigherHalf:
        mov     eax, ecx
        shl     eax, 12
        add     eax, KERNEL_LMA
    
        or      eax, PAGE_PRESENT | PAGE_RW 
    
        mov     [BootPageTable - VIRT_OFFSET + ecx*4], eax
    
        inc     ecx
        cmp     ecx, 256
        jne     .MapKernelToHigherHalf

        mov     eax, BootPageTable
        sub     eax, VIRT_OFFSET 
        or      eax, PAGE_PRESENT | PAGE_RW
    
        mov     [PageDirectory - VIRT_OFFSET + KERNEL_PDE*4], eax

        mov     eax, PageDirectory
        sub     eax, VIRT_OFFSET
        mov     cr3, eax
    
        mov     eax, cr0
        or      eax, 0x80010020 ; PG | PE | WP | NE
        mov     cr0, eax

        lea     ecx, [_PrepareKernelEntry]
        jmp     ecx

_PrepareKernelEntry:
        mov     esp, stack + STACK_SIZE 
        mov     ebp, esp ; I don't think that's necessary but whatever

        call    default_state
    
        mov     eax, [BootRegisters]
        mov     ebx, [BootRegisters + 4]

        cmp     eax, MULTIBOOT_MAGIC
        jne     .not_multiboot

        push    ebx   ; multiboot data address
        call    SystemKernelInit
    
        cli
        mov     eax, KernelReturned
        push    eax
        call    FatalError
.not_multiboot:
        ; FIXME: Somehow inform the user here.
        cli
        hlt 
        jmp .not_multiboot
