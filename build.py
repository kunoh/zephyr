#!/usr/bin/python3

import argparse
import logging
import os
import subprocess
import sys

BUILD_DIR = "build"
PATH = os.path.dirname(os.path.abspath(__file__))
WEST_CMD = ". zephyrproject/zephyr/zephyr-env.sh && west "
BOARD= {
        "ble": "nrf52840dk_nrf52840",
        "io": "mimxrt1050_evk_qspi"
        }

def run_west_cmd(cmd):
    logging.info("Running: %s", cmd)
    try:
        return subprocess.run(f"{WEST_CMD} {cmd}", check=True, shell=True, executable="/bin/bash")
    except:
        logging.error("Error running cmd %s. Did you run build.py update?", cmd)


def build_bootloader(mcu_type, board, pristine):

    cmd = f"build -b{board} -p{pristine} -d{BUILD_DIR}/{mcu_type}/bootloader zephyrproject/bootloader/mcuboot/boot/zephyr "
    if mcu_type in "io":
        config_overlay = os.path.join(PATH, "io/overlay/mimxrt1050_evk_qspi.conf")
        dts_overlay = os.path.join(PATH, "io/overlay/mimxrt1050_evk_qspi.overlay")
        cmd += f" -- -DOVERLAY_CONFIG={config_overlay} -DDTC_OVERLAY_FILE={dts_overlay}"

    ret = run_west_cmd(cmd)
    print(ret)

def build_app(mcu_type, board, pristine):
    run_west_cmd(f"build -b {board} -p {pristine} -d {BUILD_DIR}/{mcu_type}/app {mcu_type}/app")

def flash(mcu_type, board, bootloader):
    if bootloader:
        app = "bootloader"
    else:
        app = "app"
    run_west_cmd(f"build -b {board} -d {BUILD_DIR}/{mcu_type}/{app}")

def main():
    logging.basicConfig(level=logging.INFO)

    mcu_help = "Build and flash MCUs. To build io run:\n\n\tTo init west and flash io with bootloader run:\n"
    mcu_help += "\t\t#build.py init\n\t\t#build.py -t io -l\n"
    mcu_help += "\t\t#build.py -t io -l -f\n\tBuilding/flashing app:\n"
    mcu_help += "\t\t#build.py -t io\n\t\t#build.py -t io -f\n"
    mcu_help += "Ble does not require building bootloader."

    parser = argparse.ArgumentParser(description=mcu_help, formatter_class = argparse.RawTextHelpFormatter)
    parser.add_argument('-u', '--update', help='Update west modules', action='store_true')
    parser.add_argument('-t', '--type', choices=['io', 'ble'],
                        help='Build type io, ble')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='clean before build')
    parser.add_argument('-l', '--bootloader', action='store_true',
                        help='build/flash bootloader fw instead of app firmware')

    parser.add_argument('-f', '--flash', action='store_true',
                        help='flash firmware')

    args = parser.parse_args()


    if args.update:
        try:
            run_west_cmd("west update")
        except:
            logging.exception("Error doing west update")
        sys.exit(0)

    if not args.update and not args.type:
        parser.error("build and flash requires --type (-t)")

    board=BOARD.get(args.type)
    if args.clean:
        pristine="always"
    else:
        pristine="never"

    if args.bootloader:
        build_bootloader(args.type, board, pristine)
        sys.exit(0)
    if args.flash:
        flash(args.type, board, args.bootloader)
        sys.exit()

    build_app(args.type, board, pristine)


if __name__ == '__main__':
    main()
