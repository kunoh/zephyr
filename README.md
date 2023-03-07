# Introduction
MCU something
# Initialization
Initialization (one time only, executed from zephyrproject dir):

* Install Zephyr, toolchain and tools based on [Zephyr intstallation](https://docs.zephyrproject.org/3.1.0/develop/getting_started/index.html) except from west init. Instalation can also be seen in the pipelines.
* Update zephyr

``` bash
  $ ./build.py update
```
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