#!/bin/python

# Quick and dirty script to ensure our bootloader is aligned to
# a multiple to 512 bytes (The usual disk sector size) so that we can
# read whole sectors when the BIOS loads us, AND that the image size is constant
# so that bootsect/hdd/boot.asm doesn't have to guess every time.

import sys
import os
import math

SECTOR_SIZE             = 512
RESERVED_SECTORS        = 63
TOTAL_TARGET_SIZE       = SECTOR_SIZE * RESERVED_SECTORS

file            = sys.argv[1]
filesize        = os.path.getsize(file)

if filesize > TOTAL_TARGET_SIZE:
        print(f"Error: Stage 2 ({filesize} bytes) exceeds reserved space ({TOTAL_TARGET_SIZE} bytes)!")
        sys.exit(1)

padding = TOTAL_TARGET_SIZE - filesize

with open(file, "ab") as f:
        f.write(b"\x00" * padding)
