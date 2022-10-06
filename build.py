#!/usr/bin/python3

import argparse
import logging
import os
import shutil
import subprocess
import sys
from glob import glob

BUILD_DIR = "build"
PATH = os.path.dirname(os.path.abspath(__file__))
APP_DIR = "mcu-project/apps"
BOARD= {
        "ble": "nrf52840dk_nrf52840",
        "io": "mimxrt1050_evk_qspi",
        "io_display": "mimxrt1050_evk"
        }

def run_cmd(cmd):
    logging.info("Running: %s", cmd)
    try:
        return subprocess.run(cmd, check=True, shell=True, executable="/bin/bash")
    except:
        logging.error("Error running cmd %s. Try clean build or build.py update", cmd)
        sys.exit(1)

def build_bootloader(mcu_type, board, clean):

    build_dir = f"{BUILD_DIR}/{mcu_type}/bootloader"
    if clean:
        logging.info("Removing dir %s", build_dir)
        shutil.rmtree(build_dir)
    cmd = f"west build -b{board} -d{build_dir} zephyrproject/bootloader/mcuboot/boot/zephyr "
    if mcu_type in "io":
        config_overlay = os.path.join(PATH, f"{APP_DIR}/{mcu_type}/boards/bootloader/mimxrt1050_evk_qspi.conf")
        dts_overlay = os.path.join(PATH, f"mcu-project/boards/mimxrt1050_evk_qspi.overlay")
        dts_overlay = "\"" + dts_overlay + ";" + os.path.join(PATH, f"{APP_DIR}/{mcu_type}/boards/bootloader/mimxrt1050_evk_qspi.overlay" + "\"")
        cmd += f" -- -DOVERLAY_CONFIG={config_overlay} -DDTC_OVERLAY_FILE={dts_overlay}"

    ret = run_cmd(cmd)
    print(ret)

def build_app(mcu_type, board,  clean):

    build_dir = f"{BUILD_DIR}/{mcu_type}/app"
    if clean:
        logging.info("Removing dir %s", build_dir)
        shutil.rmtree(build_dir)
    run_cmd(f"west build -b {board} -d{build_dir} {APP_DIR}/{mcu_type}")

def flash(mcu_type, bootloader):
    if bootloader:
        app = "bootloader"
    else:
        app = "app"
    run_cmd(f"west flash -d {BUILD_DIR}/{mcu_type}/{app}")

def run_unit_tests(clean):

    output_dir = f"{BUILD_DIR}/unit_tests/twister-out"

    if clean:
        for directory in glob(f"{output_dir}*"):
            shutil.rmtree(directory)

    run_cmd(f"zephyrproject/zephyr/scripts/twister  -T mcu-project/tests/ -O {output_dir}")
    sys.exit(0)

def main():
    logging.basicConfig(level=logging.INFO)

    mcu_help = "Build and flash MCUs. To build io run:\n\n\tTo init west and flash io with bootloader run:\n"
    mcu_help += "\t\t#build.py update\n\t\t#build.py -t io -l\n"
    mcu_help += "\t\t#build.py -t io -l -f\n\tBuilding/flashing app:\n"
    mcu_help += "\t\t#build.py -t io\n\t\t#build.py -t io -f\n"
    mcu_help += "Ble does not require building bootloader."

    parser = argparse.ArgumentParser(description=mcu_help, formatter_class = argparse.RawTextHelpFormatter)
    parser.add_argument('-u', '--update', help='Update west modules', action='store_true')
    parser.add_argument('-t', '--type', choices=['io', 'ble', 'io_display'],
                        help='Build type')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='clean before build')
    parser.add_argument('-l', '--bootloader', action='store_true',
                        help='build/flash bootloader fw instead of app firmware')
    parser.add_argument('-f', '--flash', action='store_true',
                        help='flash firmware')
    parser.add_argument('--test', action='store_true',
                        help='Run unit tests')


    args = parser.parse_args()

    if args.test:
        run_unit_tests(args.clean)

    if args.update:
        try:
            run_cmd("west update")
        except:
            logging.exception("Error doing west update")
            sys.exit(1)
        sys.exit(0)

    if not args.update and not args.type:
        parser.error("build and flash requires --type (-t)")

    board=BOARD.get(args.type)
    if args.bootloader:
        build_bootloader(args.type, board, args.clean)
        sys.exit(0)

    build_app(args.type, board, args.clean)

    if args.flash:
        flash(args.type, args.bootloader)
        sys.exit()

if __name__ == '__main__':
    main()
