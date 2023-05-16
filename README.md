# Introduction
MCU project for IO and BLE firmware.
# Initialization
Initialization (one time only, executed from zephyrproject dir):

* Install Zephyr, toolchain and tools based on [Zephyr intstallation](https://docs.zephyrproject.org/3.3.0/develop/getting_started/index.html) except from west init. Instead of west init/update, run:

  ``` bash
    $ ./build.py --update
  ```
  This will place Zephyr in a folder called `zephyrproject` (Installation can also be seen in the pipelines).

* You might need to install the following apt packages manually:
  * protobuf-compiler

* It is recommended to setup a python virtual environment for the repo, and install the python packages from requirements into it. Because som of the required version might confilct with the ones install on your system.
* Requirements are found in:
  * docker/requirements.txt
  * zephyrproject/zephyr/scripts/requirements.txt 
* If you areworking with a 1060 board you will need to change the debugger firmware to J-Link.
  * https://community.nxp.com/t5/i-MX-RT-Knowledge-Base/Using-J-Link-with-MIMXRT1060-EVKB/ta-p/1452717
  * https://www.nxp.com/docs/en/application-note/AN13206.pdf
    * you might need LPCScrypt:
      * https://www.nxp.com/design/microcontrollers-developer-resources/lpcscrypt-v2-1-2:LPCSCRYPT?tid=vanLPCSCRYPT
      * https://www.nxp.com/docs/en/user-guide/LPCScrypt_User_Guide.pdf
    * There is a number of mismatched names in the application note when applied to the evkb:
      * DFU jumper is J12.
      * You will need to use two usb ports. The debug circuit usb port is J1 and the rest of the board will be powered by J48. Remember to move J40 to 3-4 to power the board from J48.

# Building and flashing
Using the MIMXRT1060 boards there are two ways of running the IO application firmware on the board:
* Build the application and flash as standalone.
* Build the application and Zephyr project bootloader (MCUBoot) and flash them together in separate partitions.

To use the standalone:
``` bash
  $ ./build.py -t io1060 -wb -f
```

To use the Zephyr bootloader with the IO application, first build and flash the bootloader and then the application:
``` bash
  $ ./build.py -t io1060 -l
  $ west flash -d build/io1060/bootloader/
  $ ./build.py -t io1060 -f
```
West commands work as well in the project directories. You can also run `build.py --help` for more info.

# Debug
* Install VSCode extension "Cortex-Debug"
* Add "launch.json" to `<ProjectRoot>/.vscode/` with:
  ``` json
  {
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "cwd": "${workspaceFolder}",
            "executable": "${workspaceRoot}/build/io/app/zephyr/zephyr.elf",
            "name": "app",
            "request": "launch",
            "type": "cortex-debug",
            "device": "MIMXRT1062xxx6B",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw",
            "servertype": "jlink",
            "serverpath": "/opt/SEGGER/JLink_V786c/JLinkGDBServerCLExe",
            "interface": "swd",
            "serialNumber": "", //If you have more than one J-Link probe, add the serial number here.
            //"jlinkscript":"${workspaceRoot}/BSP/SEGGER/K66FN2M0_emPower/Setup/Kinetis_K66_Target.js",
            "runToMain": true,
            "armToolchainPath": "${env:HOME}/zephyr-sdk-0.15.2/arm-zephyr-eabi/bin/",
            "svdFile": "${workspaceRoot}/MIMXRT1052.xml",
            "gdbPath": "${env:HOME}/zephyr-sdk-0.15.2/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb"
        },
    ]
  }
  ```
  Modify the zephyr-sdk, device, and JLink versions and paths to match your system.
  * MIMXRT1062xxx6B boards require zephyr-sdk >= 15.2.

# Unit tests
Zephyr's Ztest framework is used for the unit tests and run with Twister.  
All tests should be placed in mcu-project/tests and can be run with:

``` bash
build.py --test
```
See existing tests for the general structure.


* For GCC >= 11
  Compiling the Zephyr library using cpp20 is currently causing some compilation issue.
  In order to fix it copy the bits directory, placed in directory files;
  ``` bash
  cp -r files/bits zephyrproject/zephyr/boards/posix/native_posix
  ```
  * The IO project currently uses CPP14



# Working with zephyr repository

Zephyr repository can be found in zephyrproject/zephyr while all modules repositories are in zephyrproject
In order to fetch remote branches in the repositories run:
``` bash
git fetch --all
```
Zephyr repository is using zephyrproject remote.

In order to checkout different zephyr branch:
``` bash
$ cd zephyrproject/zephyr
$ git co [branch/SHA of interest]
$ west update
```

# Run in Docker
```
docker run --rm -i -v <path/to/mcu-root-directory>:/mcu -t tmesw.azurecr.io/mcu-builder:latest
cd mcu
git config --global --add safe.directory '*'
west update
./build.py -t io
```