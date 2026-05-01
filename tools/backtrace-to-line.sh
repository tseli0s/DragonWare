#!/usr/bin/env sh

# Simple script to take a series of addresses dumped by a kernel panic and run addr2line on them.
# A temporary measure, because the kernel doesn't look at the actual symbols in the backtrace, just the EBP values.

KERNEL_SYS="build/sys/kernel.sys"
ADDR2LINE="i686-elf-addr2line"

usage() {
        echo "Usage: $0 <addr1> <addr2> ..."
        echo "Addresses are dumped by the kernel in the serial port and/or the screen during a kernel fatal error."
        echo "Environment variables:"
        echo "KERNEL_SYS -> Path to the kernel ELF binary. Default: build/sys/kernel.sys"
        echo "ADDR2LINE -> addr2line binary to run. Default: i686-elf-addr2line"
}

if [ -z $1 ]; then
        usage
        exit 1
fi

for addr in "$@"; do
        func=$($ADDR2LINE -f -e $KERNEL_SYS $addr | head -n 1)
        file=$($ADDR2LINE -p -e $KERNEL_SYS $addr | awk '{print $NF}')
        echo "  $addr -> $func ($file)"
done
