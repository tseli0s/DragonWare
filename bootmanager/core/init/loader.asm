; ----------------------------------------------------------------------------------------
; FILE: loader.asm
; PURPOSE: Bootstrap code to enter protected mode, discover memory regions, modesetting et al.
; PROJECT: DragonWare Boot Manager
; DATE: 02-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

DISK_SECTOR_SIZE        equ 512
E820_MAX_ENTRIES        equ 32          ; How many memory regions we can discover
E820_SIGNATURE          equ 0x534D4150  ; Used when making e820 calls to the BIOS

; <----------------------- 16 bit code. Support for jumping to protected mode, mostly ----------------------------->
bits 16

; Since the second stage is a flat binary, this is where the first stage hands off control to us
; The tasks here are simple:
; - Discover memory regions
; - Modeset to VGA text mode
; - Disable interrupts
; - Initialize 32 bit control/segment registers
; - Enable the A20 line
; - Enable protected 32 bit mode
; - Finally jump to the C interface
section .entry
global BootloaderMain

extern __bss_start
extern __bss_end

BootloaderMain:
        cli
        xor ax, ax
        mov es, ax
        mov ds, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov sp, 0x6C00
        nop
        sti
.RecoverBootDrive:
        ; That was given by the previous stage before jumping to
        ; the second stage.
        mov [BootDevice], dl
.DetectMemoryE820:
        clc

        xor ax, ax
        mov es, ax
        lea di, [MemoryRegionsList]
        
        xor ebx, ebx
        xor bp, bp
.DiscoverLoop:
        cmp bp, E820_MAX_ENTRIES
        jae .Ready

        mov eax, 0xE820 ; The BIOS service we need, it's in the name after all
        mov ecx, 20     ; NOT 24. We need compatibility with older machines, or machines that don't support the ACPI interface.
        mov edx, E820_SIGNATURE

        int 0x15
        jc .Ready

        cmp eax, E820_SIGNATURE
        jne .Ready

        ; Some BIOSes actually return 0 length entries,
	; see https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15.2C_EAX_.3D_0xE820.
	; In case we do, simply go ahead and skip this.
        cmp dword [es:di+8],    0x0
        jne .ValidEntry

        cmp dword [es:di+12],   0x0
        je .SkipCurrent
.ValidEntry:
        add di, 20      ; BIOS wrote the entry already, di must be incremented with the offset
        inc bp
.SkipCurrent:
        test ebx, ebx
        jnz .DiscoverLoop
.Ready:
        mov word        [NumMemoryRegions],     bp
        ; Now that we have discovered all memory regions, let's change the video mode
        ; and prepare to enter protected mode.
        mov ah, 0x0                     ; Set up video mode BIOS service
        mov al, 0x3                     ; 0x3 = 80x25 colorful VGA text mode (at address 0xB8000)
        int 0x10                        ; Set up the mode before jumping to the next stage

        cli

        ; Zero out the BSS section BEFORE jumping to the C code
        ; I am not sure if the compiler does this too, but let's be sure, it's not like it's some huge
        ; processing hog to zero out a couple kilobytes
        xor ax, ax
        mov es, ax
        lea di, [__bss_start]

        ; NASM won't let me do the subtraction at assembly time so
        ; I'll just subtract it at runtime
        mov cx, __bss_end
        sub cx, __bss_start

        cld
        rep stosb

        ; Enable the A20 line so that we can address more than 1 megabyte of memory
        ; This is the fast and easy method, which isn't supported on all older systems,
        ; so TODO: support the keyboard controller method too
        in al, 0x92
        or al, 2
        out 0x92, al

        lgdt [GDTPointer]
        mov eax, cr0
        or eax, 0x00000001
        mov cr0, eax
        jmp 0x08:_InProtectedMode
.BootloaderReturned:
        cli
        hlt
        jmp .BootloaderReturned

align 8
GDTDescriptor:
        ; Null descriptor, never used and CPU faults if accessed
        NullDescriptor:
                dd 0x00000000
                dd 0x00000000
        ; Code descriptor, used when fetching instructions
        CodeDescriptor:
                dw 0xffff       ; Limit (Entire address space)
                dw 0x0000       ; Base  (All start from 0)
                db 0x0000       ; Base  (Extra eight bits)
                db 10011010b    ; Access byte
                db 11001111b    ; Flags (4 bits) + Limit (bits 16-19)
                db 0x0          ; Base (bits 24-31)
        ; Data descriptor, used for non-code accesses
        DataDescriptor:
                dw 0xffff
                dw 0x0
                db 0x0
                db 10010010b
                db 11001111b
                db 0x0
        ; 16 bit code, used when temporarily dropping to BIOS real mode
        Code16Descriptor:
                dw 0xffff
                dw 0x0000
                db 0x00
                db 10011010b
                db 00001111b
                db 0x0
        ; 16 bit data, I don't think it's actually used anywhere but safe to have it
        Data16Descriptor:
                dw 0xffff
                dw 0x0000
                db 0x00
                db 10010010b
                db 00001111b
                db 0x0
        GDTEnd:

GDTPointer:
        dw GDTEnd - GDTDescriptor - 1 ; Size of the GDT structure minus 1
        dd GDTDescriptor              ; Address of the GDT (linear)

global BootDevice
global NumMemoryRegions
BootDevice:             db 0x00
NumMemoryRegions:       dw 0x00

align 4
global MemoryRegionsList
MemoryRegionsList:      times (20 * E820_MAX_ENTRIES + 4) db 0 ; Four byte extra padding

; -------------------------------------- PROTECTED MODE CODE START -----------------------------------------------
bits 32
extern bootmain
_InProtectedMode:
        cli
        cld
        mov ax, 0x10 ; Offset 16 (0x10) in the GDT is the data descriptor
        mov ds, ax   ; cs is already set to 0x08 after the far jump
        mov ss, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        mov ebp, 0x90000 ; Our stack can now be much higher in memory
        mov esp, ebp     ; And we can use a 32 bit address

        call bootmain
.BootloaderReturned:
        cli
        hlt
        jmp .BootloaderReturned
