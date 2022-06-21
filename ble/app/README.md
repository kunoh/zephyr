# Introduction
BluetothIO prove of concept repo
# Initialization
Initialization (one time only, executed from zephyrproject dir):

* Install Zephyr, toolchain and tools as described in: [Zephyr intstallation](https://docs.zephyrproject.org/2.7.0/getting_started/index.html)
*  Set board type and build and flash mcuboot:
  ``` bash
  $ cd zephyrproject && west config build.board nrf52840dk_nrf52840
  $ west build bootloader/mcuboot/boot/zephyr/
  $ west flash
  ```

# Build and Test
Build and flash from BluetoothIO dir:

``` bash
$ #Source zephyr-env.sh to source zephyrproject ( dir can be different)
$ source ../zephyrproject/zephyr/zephyr-env.sh
$ west build -- '-DCONFIG_MCUBOOT_SIGNATURE_KEY_FILE="bootloader/mcuboot/root-rsa-2048.pem"'
$ west flash
```
or

``` bash
$ #Source zephyr-env.sh to source zephyrproject ( dir can be different)
$ . ./init.sh
$ west build
$ west flash
```
