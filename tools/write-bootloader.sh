#!/bin/sh

# Writes the DragonWare Boot Manager to the disk image given.
# More specifically, copies the first 446 bytes into the first sector, then copies the second stage
# at offset 0x200.
#
# Script should be called with:
# - Argument 1: Path to boot.img
# - Argument 2: Path to core.img
# - Argument 3: Where to write the boot manager (target disk)
#
# This script will not run in DragonWare itself. It's intended for development, where we want to preserve partition tables
# in disks but replace the boot code and so on. Since DragonWare can't be installed as of writing this script, filesystem access
# is being tested with other OSes.

BOOTIMG_PATH="$1"
COREIMG_PATH="$2"
TARGETDISK="$3"

write_log() {
        echo "[ STATUS ] $1"
}

usage() {
        echo "Usage: $0 <boot.img path> <core.img path> <target disk image>"
        exit 1
}

[ -z $1 ] && usage
[ -z $2 ] && usage
[ -z $3 ] && usage

write_bootimg() {
        dd if=$BOOTIMG_PATH of=$TARGETDISK bs=1 count=446 conv=notrunc && write_log "Wrote boot.img boot code"
}

write_coreimg() {
        dd if=$COREIMG_PATH of=$TARGETDISK bs=512 seek=1 conv=notrunc && write_log "Wrote core.img second stage at offset 0x200"
}

write_bootimg && write_coreimg && write_log "Bootloader installed to $TARGETDISK"
