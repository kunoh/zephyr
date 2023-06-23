# Introduction
PYTEST for MCU project testing.

# Initialization
* Create your environement file [my-env.json] in (fx. project folder):
(Note: It doesn't matter where you add the env-file.)

``` json
  {
   "device": {
    "mcu_serial_zephyr": "/dev/serial/by-id/usb-ZEPHYR_TM5_IO_DEVICE_5001008A231949D7-if00",
    "mcu_serial_jlink": "/dev/serial/by-id/usb-SEGGER_J-Link_000726669090-if00",
    "raspi_serial_pico": "/dev/serial/by-id/usb-Raspberry_Pi_Pico_E661640843421E21-if00",
    "mcu_communication_protocol": "usb",
    "mcu_interface": "io"
    },
    "binaries": {
        "normal": {
            "file": "/home/tm/mcu_binaries/zephyr.signed-0.1.0-master.44.bin",
            "path": "/home/tm/mcu_binaries",
            "version": "0.1.0-master.33"
        },
        "recovery": {
            "file": "/home/tm/mcu_binaries/zephyr.signed.bin",
            "path": "/home/tm/mcu_binaries",
            "version": ""
        },
        "bootloader": {
            "file": "/home/tm/mcu_binaries/zephyr.bin",
            "path": "/home/tm/mcu_binaries",
            "version": ""
        }
    },
    "paths": {
        "root_path": "/home/tm/projects/mcu",
        "test_path": "/home/tm/projects/mcu/pytest",
        "results_path": "/home/tm/projects/mcu/pytest/results",
        "flashloader_path": "/home/tm/tools/Flashloader_RT106x_1.0_GA/Tools"
    },
    "dev_mode": {
        "skip_flash": false,
        "skip_recovery": false
    }
  }
  ```
* Copy my-env.json to project mcu folder and cd:

  ``` bash
    $ cp my-env.json /home/tm/projects/mcu && cd /home/tm/projects/mcu
  ```

# Configure pytest job
* To specify firmware to be tested:
  ``` json
  {
    "binaries": {
        "normal": {
            "file": "/home/tm/mcu_binaries/YOUR-FIRMWARE",
            "path": "/home/tm/mcu_binaries",
            "version": "0.1.0-master.33"
    },
  }
  ```

* To skip Firmware flash and skip recovery machanism:
  ``` json
  {
    "dev_mode": {
        "skip_flash": true,
        "skip_recovery": true
    }
  }
  ```

* To only run specific tests use pytest.mark.custom:
in test file ```test_battery.py```:
```
   @pytest.mark.foo
    def test_batterie_pluggedIn():
        assert True

    @pytest.mark.bar
    def test_batterie_pluggedOut():
        assert True
    
    def test_batterie_LEDs():
        assert True
```


# Prepare pyton virtual environement

``` bash
    $ python3 -m venv mcu_py_venev
    $ source mcu_py_venev/bin/activate
    $ pip3 install -r $(pip_r_folder)/requirements.txt
```

# Run the test

* Run only tests marked with FOO
``` bash
    $ pytest pytest/ -v -s --junitxml pytest/results/xunit.xml --html pytest/results/report.html --envfile my-env.json -m "foo"
```
* Run only tests marked with BAR
``` bash
    $ pytest pytest/ -v -s --junitxml pytest/results/xunit.xml --html pytest/results/report.html --envfile my-env.json -m "bar"
```
* Run all tests marked with foo and not marked with bar
``` bash
    $ pytest pytest/ -v -s --junitxml pytest/results/xunit.xml --html pytest/results/report.html --envfile my-env.json -m "foo and not bar"
```
* Run all test
``` bash
    $ pytest pytest/ -v -s --junitxml pytest/results/xunit.xml --html pytest/results/report.html --envfile my-env.json
```

# Debugging test result
* Look at the console output

* Look into output files 
``` bash
    $  ll pytest/results/
```

* Flash progress in ```flash_progress.log```
* MCU debug prints in ```prints_collection.log```
* Pytest test report in ```report.html```
