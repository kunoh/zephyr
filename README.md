# Introduction
MCU something
# Initialization
Initialization (one time only, executed from zephyrproject dir):

* Install Zephyr, toolchain and tools based on [Zephyr intstallation](https://docs.zephyrproject.org/2.7.0/getting_started/index.html) except from west init. Instalation can also be seen in the pipelines.
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
