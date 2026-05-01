#!/usr/bin/python3

# A simple script that creates a QEMU machine and runs DragonWare in it (More specifically, attaches the ISO file given
# to the CDROM then instructs QEMU to boot it). Only for convenience, you can also run the executable directly and put
# all the files manually.

import argparse as ap
import subprocess
import random
import os
import string

parser = ap.ArgumentParser(description="A script to run a full QEMU virtual machine with DragonWare in it.")
parser.add_argument("-i", "--disk-image", help="Path to the ISO (disk image) to use.")
parser.add_argument("-q", "--qemu-args", help="Arguments to pass to QEMU. Use a concancated string otherwise the script won't understand.")
parser.add_argument("-d", "--disk", help="The disk to pass to QEMU. If none, no disk will be provided to the VM.")
parser.add_argument("-c", "--qemu-command", help="The QEMU command to use for the virtual machine.", type=str, default="qemu-system-i386")
parser.add_argument("-g", "--debug", help="Apply some debug flags to QEMU", action="store_true")
parser.add_argument("-n", "--no-serial-stdio", help="Do not redirect serial output to stdio", action="store_true")
parser.add_argument("-s", "--attach-gdb", help="Run DragonWare and wait for gdb in a new terminal (use `target remote :1234` in gdb)", action="store_true")
parser.add_argument("-m", "--monitor", help="Enable QEMU monitor I/O through stdio (Requires -n)", action="store_true")

args = parser.parse_args()

def random_str() -> str:
        return ''.join(random.choices(string.ascii_lowercase + string.digits, k=12))

def create_qemu_img(filename: str):
        print("note: Creating a temporary disk for the machine at " + filename + ', specify --disk DISK to override this.')
        subprocess.run([
                'qemu-img',
                'create',
                '-f',
                'raw',
                filename,
                '512M'
        ])

def spawn_qemu(args: str):
        subprocess.run(args)

def _start():
        disk_random_name = None
        disk_concat_name = None

        if args.disk_image is None:
                print("error: No disk image provided!")
                exit(-1)

        qemu_args = [
                args.qemu_command,
                '-cdrom', args.disk_image,
                '-display', 'gtk,show-tabs=on',
                '-machine', 'acpi=on',
                '-boot', 'c',
                '-m', '16M',
                '-cpu', 'pentium2',
                '-vga', 'vmware',
        ]
        debug_flags = [
                '-d', 'int,guest_errors,cpu_reset',
                '-M', 'smm=off',
                '-no-reboot', '-no-shutdown',
        ]
        # NOTE: Not intended for gdb itself, but for QEMU.
        # We need these flags to tell QEMU not to immediately proceed with the boot process,
        # but pause and wait for gdb.
        gdb_flags = [ '-s', '-S' ]
        if not args.no_serial_stdio:
                print("note: will redirect serial output to stdio")
                qemu_args.append('-serial')
                qemu_args.append('stdio')
        
        if args.monitor:
                qemu_args.append('-monitor')
                qemu_args.append('stdio')
        
        if args.disk is not None:
                newargs = [ '-drive', ('file=' + args.disk + ',if=ide')]
                for a in newargs:
                        qemu_args.append(a)
        if args.debug:
                print("note: Running with debug flags, emulation speed may be reduced.")
                for f in debug_flags:
                        qemu_args.append(f)
        else:
                print("note: Debug flags won't be appended to QEMU")

        if args.attach_gdb:
                for a in gdb_flags:
                        qemu_args.append(a)

        if args.qemu_args is not None:
                qemu_args.append(args.qemu_args)
        
        spawn_qemu(qemu_args)

if __name__ == '__main__':
        try:
                _start()
        except KeyboardInterrupt:
                print("Keyboard interrupt caught. Exiting.")
                exit(0)
