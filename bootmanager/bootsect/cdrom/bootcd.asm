; ----------------------------------------------------------------------------------------
; FILE: bootcd.asm
; PURPOSE: BIOS-compatible boot image for no emulation El Torito boot support
; PROJECT: DragonWare Boot Manager
; DATE: 03-2026
; AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
; LICENSE: GPL-v3.0-or-later, see COPYING in the toplevel directory
; ----------------------------------------------------------------------------------------

org	0x600
bits	16

SECTOR_SIZE             equ 2048        ; Size of a sector, in almost all cases, its 2048 bytes for a CD.
STAGE_2_ADDR            equ 0x7C00      ; Many BIOSes want the second stage here for some reason. So we can't use 0x8000 as before.
STACK_PTR               equ 0x6c00      ; Where to point the stack pointer register. A page below where we're loaded seems fine.
LOAD_OFFSET             equ 0x7600      ; BIOS loads us at 0x7C00, we put all addresses relative to 0x600 (see org above) so
                                        ; 0x7c00 - 0x600 = 0x7600.
REQUIRED_SYSMEM         equ 4096        ; Kilobytes of memory needed to run DragonWare. We test this below.

; Shorthand for putting the string to print in si
; and then use the PrintBIOS function to print it.
%macro PrintEarly 1
        mov     si, %1
        call    PrintBIOS
%endmacro

; This is where the BIOS will start executing. However we need to insert a jump instruction here as
; the first couple of bytes will be used by the firmware to tell us information about where we're supposed to
; be loaded.
_start:
        ; Skip the boot info table, by adding the offset manually to the jump instruction. Remember we are setting
        ; org to be 0x600, but we haven't relocated yet, so all addresses are currently pointing to garbage until
        ; we relocate ourselves!
        jmp     0x0000:(_SkipBootInformation+LOAD_OFFSET)
        times   8-($-$$) db 0x00
; The data below is filled by firmware, and we rely on this convention for the entire bootloader to work.
; Hopefully it does work across all machines. PS: This entire bootloader RELIES on LBA extensions being present.
PrimaryVolumeDescriptor:        resd 1  ; LBA
BootFileLocation:               resd 1  ; LBA too
BootFileLength:                 resd 1  ; In bytes
BootChecksum:                   resd 1  ; Not sure, we'll ignore it
_ReservedData:                  resb 40 ; Reserved "for future expansion"

; "True" entry point, actual execution begins here (Notice the jmp instruction at the beginning of the file).
; This is almost identical to the equivalent in bootmanager/bootsect/hdd/boot.asm
_SkipBootInformation:
        cli                     ; Need interrupts disabled for this part because we'll switch the stack
        
        xor     ax, ax          ; Zero out ax
        mov     ds, ax          ; And tell the processor we don't want to use any segments
        mov     es, ax
        mov     ss, ax
        mov     sp, STACK_PTR   ; Prepare a stack as well

        cld                     ; Will be used for relocation
; Since we are (so far) running at base address 0x0000:0x7c00, we need to move to 0x600 (or some other deeply low address)
; and continue execution there, because the 0x0000:0x7c00 is the target address for the second stage.
.RelocateToLowerMemory:
        mov     si, 0x7c00
        mov     di, 0x600
        mov     cx, 1024                ; Remember, the boot sector this time is 2048 bytes!
        rep     movsw                   ; Perform the copy.

        jmp     0x0000:.RelocatedCode
.RelocatedCode:
        sti
.CheckEnoughMemory:
        clc                         ; CF bug workaround, see https://wiki.osdev.org/Detecting_Memory_(x86)
        mov     ah, 0x88            ; Really old BIOS function to get high memory
        int     0x15

        ; NOTE: I intentionally don't check if CF is set here because it appears to be always set even though
        ; the routine works fine otherwise. It appears that some buggy BIOSes do that, see here:
        ; https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_AH_=_0x88

        test    ax, ax               ; ax = 0 means there was an error (eg. no high memory)...
        je      NotEnoughMemoryFail  ; ...so we assume there's no high memory at all

        cmp     ax, REQUIRED_SYSMEM  ; Now perform the actual check, to see if this machine can even load DragonWare.
        jb      NotEnoughMemoryFail  ; If not, abort the entire boot process, no point in trying.

        mov             [BootDevice], dl
        PrintEarly      BootString

        ; Now use the data given to us by the formatting tool to find the second stage on the disk and load it.
        ; Ugly, but we still modify the disk access packet manually to do this.
.SetupDiskPacket:
        ; First we need to know the LBA. We skip the first sector because it's this file
        mov     dword eax, [BootFileLocation]
        inc     dword eax
        mov     dword [DiskAccessPacket.ReadLBA], eax

        ; We already know the length more or less, but couldn't hurt to recalculate it.
        mov     dword eax, [BootFileLength]
        shr     eax, 11 ; For the uninitiated, that's division by 2048 (2 to the power of 11, yk what just read what can bit shifting do)
        dec     eax
        mov     word [DiskAccessPacket.NumSectors], ax
.LoadStage2FromDisk:
        ; Not checking for extension support explicitly. We just assume it's present. Why?
        ; 1) Almost all computers that implemented El Torito booting also supported LBA reading extensions.
        ; 2) https://forum.osdev.org/viewtopic.php?f=1&t=18911&p=146444#p146433 says:
        ; "Several BIOSes have bugs when using int 13 function 41h (EDD extensions check) on CD/DVD/etc drives."
        xor     ax, ax
        mov     ds, ax
        mov     es, ax

        mov     ah, 0x42
        mov     si, DiskAccessPacket
        mov     dl, [BootDevice]

        int     0x13
        jc      short DiskReadFail
        mov     dl, [BootDevice]
        jmp     0x0000:STAGE_2_ADDR          ; If carry flag is not set, assume success and jump to stage 2

DiskReadFail:
        PrintEarly      DiskError
        jmp             WaitForKeyAndReboot

NotEnoughMemoryFail:
        PrintEarly      NotEnoughMemory
        jmp             WaitForKeyAndReboot

WaitForKeyAndReboot:
        PrintEarly PressAnyKey
        mov     ah, 0x00            ; Wait for keypress BIOS function
        int     0x16                ; No code will be ran after this unless a key is pressed
        jmp     0xffff:0x0000       ; The BIOS copied itself at this address, so jumping there is effectively rebooting
        hlt

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

BootString:             db "Loading DragonWare Boot Manager... ", 0
BootDevice:             db 0x80
DiskError:              db "Error while trying to read from disk.", 0
NotEnoughMemory:        db "Not enough memory to boot DragonWare. You need at least 3MBs of RAM, that it appears the bootloader couldn't find. ", 0
PressAnyKey:            db "Press any key to reboot...", 0

align   4
DiskAccessPacket:
.Size:          db      0x10            ; Size of the packet, 16 bytes in this case
.Reserved:      db      0x00            ; Reserved
.NumSectors:    dw      16              ; Number of sectors to read, 16 sectors times 2048 bytes per sector is 32KBs, exactly the size of stage 2.
.MemOffset:     dw      STAGE_2_ADDR    ; Target offset within the segment
.MemSegment:    dw      0x0000          ; Target segment, always 0 because we want linear addresses
.ReadLBA:       dq      0x0             ; LBA to start reading from. We set this dynamically later (TODO)

times   2048-($-$$)     db 0x00
