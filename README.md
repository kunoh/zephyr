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


# Manual builds
Zephyrproject contains the directory that is controlled by west init and west update commands
In order to use west manually, source zephyr environment:
``` bash
$ . zephyrproject/zephyr/zephyr-env.sh
```

In order to checkout different zephyr branch:
``` bash
$ cd zephyrproject/zephyr
$ git co [branch/SHA of interest]
$ west update
$ # To push changes
$ cd ../.. && git add zephyrproject && git commit -m "Updated zephyr repo to XXX"
```
