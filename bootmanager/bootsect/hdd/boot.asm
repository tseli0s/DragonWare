; ----------------------------------------------------------------------------------------
; FILE: boot.asm
; PURPOSE: Real mode BIOS-compatible boot sector for DragonWare
; PROJECT: DragonWare Boot Manager
; DATE: 01-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

org 0x600
bits 16

STAGE_2_ADDR            equ 0x7C00      ; Where to load the second stage (linear address, handled separately)
STACK_PTR               equ 0x6c00      ; A page below where the BIOS loaded us at (Acting like pages are a thing for this comment's purposes)
PART_TABLE              equ 0x1BE       ; Offset to the partition table
PART_ENTRY_SIZE         equ 16          ; Size of each MBR entry
REQUIRED_SYSMEM         equ 4096        ; Kilobytes of memory needed to run DragonWare. We test this below.

; Shorthand for putting the string to print in si
; and then use the PrintBIOS function to print it.
%macro PrintEarly 1
        mov     si, %1
        call    PrintBIOS
%endmacro

; BIOS has loaded us into address 0x0000:0x7c00. We only have 446 bytes to do whatever
; we want, the next 64 bytes is the MBR table and the last two bytes are the bootable signature.
; NOTE: Addresses 0x0-0x3ff contain the IVT for the BIOS. There's also some BIOS stuff following,
; so assume everything below 0x600 (where we relocate) is just unusable.
_start:
        cli
        cld
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov sp, STACK_PTR     ; The stack grows downwards on x86

        ; Relocate the boot code to 0x600, so that we can load the VBR of the active
        ; partition to 0x7c00 without overwriting the executing code.
.RelocateToLowerAddress:
        mov si, 0x7C00
        mov di, 0x600
        mov cx, 256
        rep movsw

        jmp 0x0000:BootSectorCode
BootSectorCode:
        sti

        ; Detect the amount of high memory in the system. If it's not enough, inform the user and stop booting.
        ; This will save us some extra checking in later stages, the bootloader can "guarantee" that there's enough memory.
.CheckEnoughMemory:
        clc                     ; CF bug workaround, see https://wiki.osdev.org/Detecting_Memory_(x86)
        mov ah, 0x88            ; Really old BIOS function to get high memory
        int 0x15

        ; NOTE: I intentionally don't check if CF is set here because it appears to be always set even though
        ; the routine works fine otherwise. It appears that some buggy BIOSes do that, see here:
        ; https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_AH_=_0x88

        test ax, ax             ; ax = 0 means there was an error (eg. no high memory)...
        je NotEnoughMemoryFail  ; ...so we assume there's no high memory at all

        cmp ax, REQUIRED_SYSMEM ; Now check how much memory is there.
        jb NotEnoughMemoryFail  ; If there's less than required, this machine probably can't run DragonWare

.Continue:
        mov [BootDevice], dl  ; Remember what device we booted off of, the BIOS puts it in dl
        PrintEarly BootString ; Boot messages look cool.

        cmp byte [BootFromActive], 0    ; Test if the bootloader is configured to boot off of the active partition
        je .Stage2InDisk                ; If not, just try to read stage 2 as usual.

        lea si, [PART_TABLE + 0x0600]
        mov cx, 4

        ; Try to see if there's an active partition. If so, boot off of that.
.FindActivePartition:
        cmp byte [si], 0x80
        je .ActivePartitionDetected
        add si, PART_ENTRY_SIZE
        loop .FindActivePartition
        
        ; No active partition was found. Therefore, go to the good old way of reading the next stage
        ; from raw sectors.
        jmp .Stage2InDisk
.ActivePartitionDetected:
        ; First the disk access packet must be modified with the actual first LBA of this partition
        ; See VBRDiskAccessPacket definition for more details.
        mov ax, [si+8]
        mov dx, [si+10]
        mov [VBRDiskAccessPacket+8], ax
        mov [VBRDiskAccessPacket+10], dx

        ; Then it's just a standard read as we do below.
        mov dl, [BootDevice]
        mov si, VBRDiskAccessPacket
        mov ah, 0x42
        int 0x13
        jc DiskReadFail

        jmp 0x0000:0x7C00
.Stage2InDisk:
        ; Okay, we're now ready to load the next stage. We assume the next 63 sectors
        ; are reserved for our bootloader. If not, then we're screwed. The good news: Almost
        ; every decent partitioning tool leaves some space between LBAs 0-2048 that we can use for ourselves.
        ; The bootloader must be ALWAYS installed there, right after this boot code is installed. At least
        ; until the bootloader works properly.
	;
        ; Obviously the correct way is to actually read the partition table. But what happens if the disk is unpartitioned?
        ; So for now, just assume that there IS enough space. And of course, when the bootloader will be installed, we should preserve the
        ; partition table, but we can wait on that for now.

        ; Before actually reading the next stage, we need to check whether the BIOS can actually
        ; load it using LBA reading.
        ; TODO: Have a CHS version if this isn't supported
        mov dl,         [BootDevice]
        jmp             CheckEDDExtensionsThenLoadStage2

; Check whether the firmware supports LBA reading of sectors (present in almost every IA-32 computer since 1998)
; If it fails, it jumps to DiskReadFail.
CheckEDDExtensionsThenLoadStage2:
        mov ah, 0x41
        mov bx, 0x55AA

        int 0x13
        jc NoExtensions

        cmp bx, 0xAA55
        jne NoExtensions

        test cx, 0x0001
        jz NoExtensions

;       <----------------- The BIOS does support EDD extensions, so let's start reading ----------------------------->        
        xor ax, ax
        mov ds, ax
        mov es, ax

        mov ah, 0x42
        mov si, DiskAccessPacket
        mov dl, [BootDevice]

        int 0x13
        jc short DiskReadFail

        mov dl, [BootDevice]
        jmp 0x000:STAGE_2_ADDR          ; If carry flag is not set, assume success and jump to stage 2

NoExtensions:
        PrintEarly NoExtensionsString
        jmp WaitForKeyAndReboot

DiskReadFail:
        PrintEarly DiskError
        jmp WaitForKeyAndReboot

NotEnoughMemoryFail:
        PrintEarly NotEnoughMemory
        jmp WaitForKeyAndReboot

PrintBIOS:
        lodsb
        test al, al
        jz .done
        mov ah, 0x0E
        mov bh, 0
        mov bl, 0x07
        int 0x10
        jmp PrintBIOS
.done:
        ret

WaitForKeyAndReboot:
        PrintEarly PressAnyKey
        mov ah, 0x00            ; Wait for keypress BIOS function
        int 0x16                ; No code will be ran after this unless a key is pressed
        jmp 0xffff:0x0000       ; The BIOS copied itself at this address, so jumping there is effectively rebooting
        hlt

BootString:             db "Loading DragonWare Boot Manager... ", 0
NoExtensionsString:     db "Unsupported BIOS. ", 0
DiskError:              db "Disk error. ", 0
PressAnyKey:            db "Press any key to reboot.", 0
NotEnoughMemory:        db "Not enough memory to boot DragonWare! ", 0
BootDevice:             db 0x0          ; Reserve a single byte to save dl (boot medium index) to
BootFromActiveLabel:    db "DRV"        ; Instead of recompiling the bootloader every time, looking right after DRV will get you the value of BootFromActive,
                                        ; so changing it is literally searching for the "DRV" string and writing a value right after.
BootFromActive:         db 0            ; If set to 1, then the bootloader will boot from the active partition instead of trying to load stage 2.

; The disk access packet given to the BIOS to read the next stage.
DiskAccessPacket: 
        db 0x10         ; The size of the packet, must always be 16
        db 0x0          ; Reserved, must always be 0
        dw 63           ; The amount of sectors to read. We need/reserve 63 sectors (this file is the first sector which must be ignored)
        dw STAGE_2_ADDR ; offset within the segment (segmented memory model: segment * 16 + offset = linear address, so 0x0 * 16 + 0x8000 = 0x8000)
        dw 0x0000       ; segment, see above
        dd 1            ; LBA low 32 bits (we start at LBA 1, LBA 0 is this file)
        dd 0            ; LBA high 32 bits (if we needed a 64 bit address, but we dont so 0)

VBRDiskAccessPacket:
        db 0x10
        db 0
        dw 1
        dw 0x7C00
        dw 0x0000
        dq 0            ; This will be set at runtime.

; Explicitly fill the MBR table with zeros. Partitioning tools should write it.
; The separate padding here is because we don't want our code overflowing into the
; MBR region.
times 446-($-$$) db 0x00

; Pad the rest of the sector with zeros,
; so that we can reach 512 bytes (A single sector), that the BIOS
; needs to boot us.
times 510-($-$$) db 0

BootMagicFlag:
        dw 0xAA55 ; Bootable signature for BIOS
