#!/usr/bin/env python3

# A simple script to launch Bochs with DragonWare in it.
# For the options, edit bochsdbg.cfg

import subprocess
import os
import argparse as ap

a = ap.ArgumentParser()
a.add_argument('-b', "--bochs-command", help="Path to the bochs executable", type=str, default="bochs")
a.add_argument('-c', "--config", help="Config file to supply to bochs", type=str, default="./tools/bochsdbg.cfg")
a.add_argument('-g', "--gdbcompat", help="Use the Bochs build with the gdbstub config.", action="store_true")
args = a.parse_args()

def start_bochs(bochsexec):
        print("running from: " + os.getcwd())
        print("bochsexec: ", bochsexec)
        subprocess.run(bochsexec)

if __name__ == '__main__':
        cfg = args.config
        bochscmd = args.bochs_command
        if args.gdbcompat:
                cfg = "./tools/bochsgdb.cfg"
                bochscmd = "bochsgdb"
        bochsexec = [
                        bochscmd,
                        '-qf', cfg,
                ]
        if not args.gdbcompat:
                bochsexec.append('-debugger')

        try:
                start_bochs(bochsexec)        
        except KeyboardInterrupt:
                exit(0)
