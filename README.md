# Introduction
MCU something
# Initialization
Initialization (one time only, executed from zephyrproject dir):

* Install Zephyr, toolchain and tools based on [Zephyr intstallation](https://docs.zephyrproject.org/3.1.0/develop/getting_started/index.html) except from west init. Instalation can also be seen in the pipelines.
* Update zephyr



``` bash
  $ ./build.py update
```

* You might need to install the following apt packages manually:
  * protobuf-compiler

* It is recommended to setup a python virtual environment for the repo, and install the python packages from requirements into it. Because som of the required version might confilct with the ones install on your system.
  * make sure you get protobuf==3.19.0  as newer versions currently dont compile (12-03-2023).

* If you areworking with a 1060 board you will need to change the debugger firmware to J-Link.
  * https://community.nxp.com/t5/i-MX-RT-Knowledge-Base/Using-J-Link-with-MIMXRT1060-EVKB/ta-p/1452717
  * https://www.nxp.com/docs/en/application-note/AN13206.pdf
    * you might need LPCScrypt:
      * https://www.nxp.com/design/microcontrollers-developer-resources/lpcscrypt-v2-1-2:LPCSCRYPT?tid=vanLPCSCRYPT
      * https://www.nxp.com/docs/en/user-guide/LPCScrypt_User_Guide.pdf

* For IO build and flash bootloader:

``` bash
  $ ./build.py -t io -b
  $ ./build.py -t io -b flash
```


# Build and Test
Build and flash using ./build.py
Run ./build.py --help to get options

West commands work as well in the project directories
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

# Unit tests
Zephyr's ztest framework is used for the unit tests

For GCC >= 11
Compiling the Zephyr library using cpp20 is currently causing some compilation issue.
In order to fix it copy the bits directory, placed in directory files;
``` bash
cp -r files/bits zephyrproject/zephyr/boards/posix/native_posix
```

All tests should be placed in mcu-project/tests
Tests can be run as

``` bash
build.py --test
```

# Run in Docker
```
docker run --rm -i -v <path/to/mcu-root-directory>:/mcu -t tmesw.azurecr.io/mcu-builder:latest
cd mcu
git config --global --add safe.directory '*'
west update
./build.py -t io
```