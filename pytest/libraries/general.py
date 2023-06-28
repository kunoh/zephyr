import subprocess, time
import os, pathlib
from typing import Optional
from libraries.serial_if import SerialMcu
from libraries.sec_prov_sdk import SecProvSDK
from libraries.switchbox_if import SwitchboxIF

HASH_LEN = 64
class General():
    def get_list_of_images_from_str(images_str:str=""):
        count_slot = -1
        last_slot = count_slot
        count_elem = -1
        image_slots = []
        for line in images_str.splitlines():
            if "Images:" in line:
                pass
            elif "image=" and "slot=" in line:
                count_slot += 1
                print(f"Found Image::Slot: {count_slot}")
                image_slots.append({})
                last_slot = count_slot
                for x in line.strip().split():
                    image_slots[count_slot][x.split("=")[0].strip()] = x.split("=")[1].strip()
            elif count_slot != -1 and last_slot == count_slot:
                count_elem += 1
                image_slots[count_slot][line.strip().split(":")[0].strip()] = line.strip().split(":")[1].strip()
                if count_elem == 3:
                    last_slot = -1
                    count_elem = -1
        return image_slots

    def firmware_upgrade(env:dict={}, special_fw:Optional[dict]=None):
        if env['dev_mode']['skip_flash']:
            print(f"Skipped FW bupdate")
            return True
        fw_file = ""
        if special_fw:
            fw_file = special_fw
        else:
            fw_file = env['binaries']['normal']['file']
        print(f"Updating with fw {fw_file}")
        if not os.path.exists(fw_file):
            print(f"FW binary does not exist")
            return False

        base_cmd = '{}/go/bin/mcumgr --conntype serial --connstring "{},baud=115200"'.format(pathlib.Path.home(), env['device']['mcu_serial_zephyr'])

        exec_cmd_list_image = base_cmd + " image list"
        print(f"{exec_cmd_list_image}")
        try:
            out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT, timeout=10)
        except Exception as e:
            print(f"{e}")
            return False
        assert out.decode() is not None
        print(out.decode())
        image_slots = General.get_list_of_images_from_str(out.decode())
        if len(image_slots) == 2 and image_slots[1]['hash'] == image_slots[0]['hash']:
            print(f"The last firmware update was skipped")
            # conitnue with new fw update
        flash_progress_file = os.path.join(env['paths']['results_path'], "flash_progress.log")
        exec_cmd_fw_upgrade = base_cmd + " image upload {} > {}".format(fw_file, flash_progress_file)
        exec_cmd_fw_upgrade = '{}/projects/mcumgr-client/target/release/mcumgr-client --mtu 256 --device {} upload {} > {}' \
            .format(pathlib.Path.home(), env['device']['mcu_serial_zephyr'], fw_file, flash_progress_file)
        print(f"{exec_cmd_fw_upgrade}")
        try:
            out = subprocess.check_output(exec_cmd_fw_upgrade, shell=True, stderr=subprocess.STDOUT, timeout=300)
        except subprocess.CalledProcessError as oute:
            print("error code", oute.returncode, oute.output)
            return False

        print(f"{exec_cmd_list_image}")
        try:
            out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT, timeout=10)
        except Exception as e:
            print(f"{e}")
            return False
        assert out.decode() is not None
        print(out.decode())
        image_slots = General.get_list_of_images_from_str(out.decode())
        assert len(image_slots) == 2

        new_fw_hash = image_slots[1]['hash']
        old_fw_hash = image_slots[0]['hash']
        if new_fw_hash == old_fw_hash:
            print(f"Firmware already installed: {fw_file}")
            return True

        if len(new_fw_hash) != HASH_LEN:
            print(f"FW hash len does not match")
            return False
        exec_cmd_fw_test = base_cmd + " image test {}".format(new_fw_hash)
        print(f"{exec_cmd_fw_test}")
        try:
            out = subprocess.check_output(exec_cmd_fw_test, shell=True, stderr=subprocess.STDOUT, timeout=60)
        except Exception as e:
            print(f"{e}")
            return False
        assert out.decode() is not None
        print(out.decode())

        image_slots = General.get_list_of_images_from_str(out.decode())
        assert len(image_slots) == 2
        assert image_slots[1]['flags'] == "pending"

        exec_cmd_reset = base_cmd + " reset"
        print(f"{exec_cmd_reset}")
        try:
            out = subprocess.check_output(exec_cmd_reset, shell=True, stderr=subprocess.STDOUT, timeout=60)
        except Exception as e:
            print(f"{e}")
            return False
        assert out.decode() is not None
        print(out.decode())
        assert out.decode().strip() == "Done"

        timeout = 120
        time_start = time.time()
        print(f"Validate new firmware hash (Wait for only ONE Image::Slot)...")
        while timeout > (time.time() - time_start):
            if SerialMcu(port=env['device']['mcu_serial_zephyr']).port_is_available():
                try:
                    out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT, timeout=10)
                    assert out.decode() is not None
                    image_slots = General.get_list_of_images_from_str(out.decode())
                    print(image_slots)
                    if len(image_slots) == 1:
                        print(out.decode())
                        assert len(image_slots[0]['hash']) == HASH_LEN and image_slots[0]['hash'] == new_fw_hash
                        return True
                except Exception as e:
                    print(f"{e}")
                finally:
                    print(f"Try again in 5 seconds...")
                    time.sleep(5)
            else:
                time.sleep(5)

        print(f"FW update failed: Could not validate new firmware hash within {timeout} secs ")
        return False

    def get_all_markers(*markers_list):
        def all_markers(func):
            for mark in reversed(markers_list):
                func = mark(func)
            return func
        return all_markers

    def recover_mcu(env):
        if env['dev_mode']['skip_recovery']:
            print(f"Skipped MCU recovery")
            return True

        print("Start MCU recovery")

        switch = SwitchboxIF(env['device']['raspi_serial_pico'])
        if switch.set_mcu_boot_mode("bootloader") is False:
            print("Failed to enter MCU BOOTLOADER mode")
            return False

        sdk = SecProvSDK(tool_path=env['paths']['flashloader_path'])

        # Flash flashloader, config and update recovery fw
        if sdk.write_ivt_flashloader_to_internal_ram() is False:
            print("Failed to write ivt flashloader")
            return False
        time.sleep(1)
        if sdk.boot_ivt_flashloader_image() is False:
            print("Failed to write ivt flashloader image")
            return False
        time.sleep(1)
        if sdk.validate_blhost_connetcion() is False:
            print("Failed to validate blhost connection")
            return False
        time.sleep(1)
        if sdk.write_optionblock_to_sram() is False:
            print("Failed to write option block to SRAM")
            return False
        time.sleep(1)
        if sdk.config_qspi_using_optionblock() is False:
            print("Failed to configure QSPI option block")
            return False
        time.sleep(1)
        if sdk.erase_all() is False:
            print("Failed to erase Flash all sector")
            return False
        time.sleep(1)
        if sdk.flash_mcu_default_bootl(boot_file=env['binaries']['bootloader']['file']) is False:
            print(f"Failed to flash MCU default boot firmware: {env['binaries']['bootloader']['file']}")
            return False
        time.sleep(1)
        if sdk.flash_mcu_default_app(firmware_file=env['binaries']['recovery']['file']) is False:
            print(f"Failed to flash MCU default app firmware: {env['binaries']['recovery']['file']}")
            return False
        time.sleep(1)

        # bring MCU to normal mode
        if switch.set_mcu_boot_mode("app", hard_reset=False) is False:
            print("Failed to set MCU APP mode")
            return False

        # Reset MCU
        if sdk.reset_mcu() is False:
            print(f"Failed to soft reset MCU: Try hard reset")
            if switch.reset_hard_mcu() is False:
                print("Also hard reset failed")
                return False
        else:
            if switch.validate_usb("app") is False:
                print(f"Failed to validate USB in mode APP / ZEPHYR")
                return False

        return True