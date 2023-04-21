import subprocess, time
import os, pathlib
from libraries.serial import Serial

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

    def firmware_upgrade(env:dict={}):
        print(f"Updating with fw {env['binary']['file']}")
        if not os.path.exists(env['binary']['file']):
            print(f"FW binary does not exist")
            return False
    
        base_cmd = '{}/go/bin/mcumgr --conntype serial --connstring "{},baud=115200"'.format(pathlib.Path.home() ,env['device']['mcu_serial_zephyr'])

        exec_cmd_list_image = base_cmd + " image list"
        print(f"{exec_cmd_list_image}")
        out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT)
        assert out.decode() is not None
        print(out.decode())
        image_slots = General.get_list_of_images_from_str(out.decode())
        assert len(image_slots) == 1

        exec_cmd_fw_upgrade = base_cmd + " image upload {}".format(env['binary']['file'])
        print(f"{exec_cmd_fw_upgrade}")
        out = subprocess.check_output(exec_cmd_fw_upgrade, shell=True, stderr=subprocess.STDOUT)
        assert out.decode() is not None
        print(out.decode())
        print(f"{exec_cmd_list_image}")
        out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT)
        assert out.decode() is not None
        print(out.decode())
        image_slots = General.get_list_of_images_from_str(out.decode())
        assert len(image_slots) == 2

        new_fw_hash = image_slots[1]['hash']
        old_fw_hash = image_slots[0]['hash']
        if new_fw_hash == old_fw_hash:
            print(f"Firmware already installed: {env['binary']['file']}")
            return True

        if len(new_fw_hash) != HASH_LEN:
            print(f"FW hash len does not match")
            return False
        exec_cmd_fw_test = base_cmd + " image test {}".format(new_fw_hash)
        print(f"{exec_cmd_fw_test}")
        out = subprocess.check_output(exec_cmd_fw_test, shell=True, stderr=subprocess.STDOUT)
        assert out.decode() is not None
        print(out.decode())
           
        image_slots = General.get_list_of_images_from_str(out.decode())
        assert len(image_slots) == 2
        assert image_slots[1]['flags'] == "pending"

        exec_cmd_reset = base_cmd + " reset"
        print(f"{exec_cmd_reset}")
        out = subprocess.check_output(exec_cmd_reset, shell=True, stderr=subprocess.STDOUT)
        assert out.decode() is not None
        print(out.decode())
        assert out.decode().strip() == "Done"

        timeout = 60
        time_start = time.time()
        print(f"Validate new firmware hash (Wait for only ONE Image::Slot)...")
        while timeout > (time.time() - time_start):
            if Serial.port_is_available(env['device']['mcu_serial_zephyr']):
                try: 
                    out = subprocess.check_output(exec_cmd_list_image, shell=True, stderr=subprocess.STDOUT)
                    assert out.decode() is not None
                    image_slots = General.get_list_of_images_from_str(out.decode())
                    if len(image_slots) == 1:
                        print(out.decode())
                        assert len(image_slots[0]['hash']) == HASH_LEN and image_slots[0]['hash'] == new_fw_hash
                        return True
                except Exception as e:
                    print(f"{e}")
                    print(f"Try again in 20 seconds...")
                    time.sleep(20)
            else: 
                time.sleep(10)
        
        print(f"FW update failed: Could not validate new firmware hash within {timeout} secs ")
        return False

    def compose_decos(*decos):
        def composition(func):
            for deco in reversed(decos):
                func = deco(func)
            return func
        return composition
    
