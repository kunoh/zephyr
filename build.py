#!/usr/bin/python3

import argparse
import logging
import os
import shutil
import subprocess
import sys
import re
from glob import glob
from pathlib import Path

BUILD_DIR = "build"
PATH = os.path.dirname(os.path.abspath(__file__))
BOOT_DIR = "mcu-project/bootloaders"
APP_DIR = "mcu-project/apps"
BOARD = {
        "ble": "nrf52840dk_nrf52840",
        "io1050": "mimxrt1050_evk_qspi",
        "io1060": "mimxrt1060_evk"
        }
PROJECT_PATH = {
            "io": "mcu-project/apps/io",
            "ble": "mcu-project/apps/ble",
            "modules": "mcu-project/modules"
           }
DOXYGEN_BUILD_DIRS = {
                    "io": "Doxygen_io",
                    "ble": "Doxygen_ble",
                    "modules": "Doxygen_modules"
                    }

def run_cmd(cmd):
    logging.info("Running: %s", cmd)
    try:
        return subprocess.run(cmd, check=True, shell=True, executable="/bin/bash")
    except:
        logging.error("Error running cmd %s. Try clean build or build.py update", cmd)
        sys.exit(1)

def add_to_overlay(current_overlay, new_overlay):
    if not current_overlay:
        return os.path.join(PATH, new_overlay)
    # Remove any qoutes from both start and end of existing overlay string
    if current_overlay[0] == '"':
        current_overlay = current_overlay[1:]
    if current_overlay[-1] == '"':
        current_overlay = current_overlay[:-1]
    return "\"" + current_overlay + ";" + os.path.join(PATH, new_overlay + "\"")

def build_bootloader(mcu_type, board, clean):

    build_dir = f"{BUILD_DIR}/{mcu_type}/bootloader"
    if clean:
        logging.info("Removing dir %s", build_dir)
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)

    cmd = f"west build -b {board} -d {build_dir} zephyrproject/bootloader/mcuboot/boot/zephyr "
    if "io" in mcu_type:
        mcu_type = "io"
        config_overlay = add_to_overlay(None, f"{BOOT_DIR}/{mcu_type}/boards/{board}.conf")
        dts_overlay = add_to_overlay(None, f"mcu-project/boards/{board}.overlay")
        dts_overlay = add_to_overlay(dts_overlay, f"{BOOT_DIR}/{mcu_type}/boards/{board}.overlay")
        dts_overlay = add_to_overlay(dts_overlay, "zephyrproject/bootloader/mcuboot/boot/zephyr/app.overlay")
        cmd += f" -- -DOVERLAY_CONFIG={config_overlay} -DDTC_OVERLAY_FILE={dts_overlay}"

    ret = run_cmd(cmd)
    print(ret)

def build_app(mcu_type, board, clean, release, without_bootloader, version, development):

    build_dir = f"{BUILD_DIR}/{mcu_type}/app"
    if clean:
        logging.info("Removing dir %s", build_dir)
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)

    overlay = ""
    extra_args = ""
    if "io" in mcu_type:
        mcu_type = "io"
        if development:
            config_overlay = add_to_overlay(None, f"{APP_DIR}/{mcu_type}/development.conf")
        else:
            config_overlay = add_to_overlay(None, f"{APP_DIR}/{mcu_type}/{board}.conf")
        config_overlay = add_to_overlay(config_overlay, f"{APP_DIR}/{mcu_type}/logging.conf")
        if not release:
            config_overlay = add_to_overlay(config_overlay, f"{APP_DIR}/{mcu_type}/debug.conf")
        if not without_bootloader:
            config_overlay = add_to_overlay(config_overlay, f"{APP_DIR}/{mcu_type}/bootloader.conf")
        dts_overlay = add_to_overlay(None, f"mcu-project/boards/{board}.overlay")
        dts_overlay = add_to_overlay(dts_overlay, f"{APP_DIR}/{mcu_type}/boards/{board}.overlay")
        if version:
            extra_args = f'-DCONFIG_MCUBOOT_EXTRA_IMGTOOL_ARGS="\\"--version {version}\\""'
        overlay = f"-- -DOVERLAY_CONFIG={config_overlay} -DDTC_OVERLAY_FILE={dts_overlay} {extra_args}"

    run_cmd(f"west build -b {board} -d {build_dir} {APP_DIR}/{mcu_type} {overlay}")

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

def format():
    clang_format_bin = shutil.which('clang-format-12')
    if not clang_format_bin:
        clang_format_bin = shutil.which('clang-format-13')

    if not clang_format_bin:
        logging.error('Cannot find clang-format. Run: sudo apt install clang-format')
        sys.exit(1)

    root_dir = Path(PATH)
    dirs_to_format = [
        root_dir / 'mcu-project' / 'apps',
        root_dir / 'mcu-project' / 'modules'
        ]

    for d in dirs_to_format:
        if run_cmd(f"find {d} -iname *.h -o -iname *.cpp | xargs {clang_format_bin} -style=file -i").returncode != 0:
            logging.error('Failed to format code')
            sys.exit(1)

    logging.info('Done')


def gen_doxygen(proj):
    # Remove existing dirs
    root_dir = Path(PATH)
    dir_to_doc = root_dir / PROJECT_PATH.get(proj) / "Doxygen"
    rel_output_dir = f"{BUILD_DIR}/{DOXYGEN_BUILD_DIRS.get(proj)}"
    abs_output_dir = root_dir / rel_output_dir

    if not os.path.exists(abs_output_dir):
        if run_cmd(f"mkdir -p {rel_output_dir}").returncode != 0:
            logging.error(f"Failed to create Doxygen output directory: {abs_output_dir}")
            sys.exit(1)

    if run_cmd(f"cd {dir_to_doc} && doxygen").returncode != 0:
        logging.error(f"Failed to generate doxygen docs for {dir_to_doc}")
        sys.exit(1)

    logging.info(f"Ran doxygen for {dir_to_doc}")


def main():
    logging.basicConfig(level=logging.INFO)

    mcu_help = "Build and flash MCUs. To build io run:\n\n\tTo init west and flash io with bootloader run:\n"
    mcu_help += "\t\t#build.py update\n\t\t#build.py -t io -l\n"
    mcu_help += "\t\t#build.py -t io -l -f\n\tBuilding/flashing app:\n"
    mcu_help += "\t\t#build.py -t io\n\t\t#build.py -t io -f\n"
    mcu_help += "Ble does not require building bootloader."

    parser = argparse.ArgumentParser(description=mcu_help, formatter_class = argparse.RawTextHelpFormatter)
    parser.add_argument('-u', '--update', help='Update west modules', action='store_true')
    parser.add_argument('-t', '--type', choices=['io1050', 'io1060', 'ble'],
                        help='Build type')
    parser.add_argument('-c', '--clean', action='store_true',
                        help='Clean before build')
    parser.add_argument('-l', '--bootloader', action='store_true',
                        help='Build/flash bootloader fw instead of app firmware')
    parser.add_argument('-f', '--flash', action='store_true',
                        help='Flash firmware')
    parser.add_argument('-r', '--release', action='store_true',
                        help='Build firmware for release')
    parser.add_argument('-d', '--development', action='store_true',
                        help='Build with drivers disabled')
    parser.add_argument('-fo', '--flash-only', action='store_true',
                        help='Just flash, no build.')
    parser.add_argument('-wb', '--without-bootloader', action='store_true',
                        help='Build app without the need of a bootloader')
    parser.add_argument('--test', action='store_true',
                        help='Run unit tests')
    parser.add_argument('--format', action='store_true',
                        help='Format code using clang')
    parser.add_argument('--document', choices=['io', 'ble', 'modules'],
                        help='Build Doxygen documentation')
    parser.add_argument('-v', '--version', type=str, default='0.0.0+0',
                        help='Inject a version into the signed binary, format; <major>.<minor>.<patch>[+build]')

    args = parser.parse_args()

    if args.format:
        format()
        sys.exit(0)

    if args.document:
        gen_doxygen(args.document)
        sys.exit(0)

    if args.test:
        run_unit_tests(args.clean)
        sys.exit(0)

    if args.update:
        try:
            run_cmd("west update")
        except:
            logging.exception("Error doing west update")
            sys.exit(1)
        sys.exit(0)

    if not args.type:
        parser.error("build and flash requires --type (-t)")

    mcu_type = args.type
    board=BOARD.get(mcu_type)
    if not args.flash_only:
        if args.bootloader:
            build_bootloader(args.type, board, args.clean)
        else:
            build_app(mcu_type, board, args.clean, args.release, args.without_bootloader, args.version, args.development)
    else:
        args.flash = True

    if args.flash:
        flash(mcu_type, args.bootloader)
        sys.exit()

if __name__ == '__main__':
    main()
