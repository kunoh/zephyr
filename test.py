import sys
import subprocess
import time

filename = "/home/kuno/git/mcu-zephyr-master/zephyrproject/zephyr/samples/hello_world/src/main.cpp"
reset_counter = 0

def do_add_print():
    with open(filename, "r") as f:
        main = f.read()

    main = main.replace("PRINT", 'PRINT\n\tprintk("Test");')

    with open(filename, "w") as f:
        f.write(main)

def do_build():
    print("Build")
    subprocess.run(['west build -p -b mimxrt1060_evk samples/hello_world/ -d build'], shell=True)

def do_flash():
    print("Flash")
    for retry in range(5):
        try:
            subprocess.run(['west flash'], shell=True, check=True)
            break
        except Exception as e:
            print(f"Failed to flash, retrying... {retry}, err {e}")

def do_upload():
    global reset_counter
    print("Upload")
    while (1):
        try:
            subprocess.run(['/home/kuno/git/epd-master/build-host/cmake-build/bin/smp_cli /dev/serial/by-id/usb-ZEPHYR_DEVICE_500100EE2A2449D7-if00 upload /home/kuno/git/mcu/build/io1060/app/zephyr/zephyr.signed.bin'], shell=True, check=True, timeout=15)
            reset_counter = 0
            break
        except Exception as e:
            print(f"Failed {e}")
            reset_counter += 1
            do_flash()
            time.sleep(1)

        if reset_counter == 3:
            sys.exit(1)

while (1):
    do_add_print()

    do_build()

    do_flash()

    time.sleep(1)

    do_upload()
