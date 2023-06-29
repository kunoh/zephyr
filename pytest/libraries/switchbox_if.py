from libraries.serial_if import SerialMcuAsync
import time, subprocess

class SwitchboxIF:
    def __init__(self, ser_port="",) -> None:
        self._ser = SerialMcuAsync(ser_port)

    def validate_usb(self, mode=None):
        time.sleep(1)
        if not mode:
            print("No usb mode specified")
            return False
        if mode == "app":
            regx = "ZEPHYR"
        if mode == "bootloader":
            regx = "NXP"
        
        try:
            out = subprocess.check_output(f"lsusb | awk '/{regx}/'", shell=True, stderr=subprocess.STDOUT, timeout=10)
            print(str(out.decode()))
            if mode == "app" and "ZEPHYR TM5 IO DEVICE" in str(out.decode()):
                return True
            elif mode == "bootloader" and "NXP Semiconductors" in str(out.decode()):
                return True
            else:
                print(f"Usb device not found in mode {mode}/{regx}")
                return False
        except Exception as e:
            print(f"{e}")
            return False

    def mcu_boot_mode_set(self, mode="bootl", response=""):   
        pin3_line = None
        pin4_line = None
        for line in response.splitlines():
            if len(line) != 0:
                print(line) if "Raspberry" not in line else print("--" * len(line)) 
                if "boot_pin_4" in line:
                    pin4_line = line
                if "boot_pin_3" in line:
                    pin3_line = line

        if not pin3_line or not pin4_line:
            print("Failed to get PIN 3/4 configuration")
            return False

        if mode == "bootloader":
            if ("boot_pin_4" and "high" in pin4_line) and ("boot_pin_3" and "low" in pin3_line):
                return True
        elif mode == "app":
            if ("boot_pin_4" and "low" in pin4_line) and ("boot_pin_3" and "high" in pin3_line):
                return True
        else:
            print("No mode specified")
        
        return False

    def set_mcu_boot_mode(self, mode=None):
        if not mode:
            print("Boot mode should be specified")
            return False
        
        resp = self._ser.async_send_read_message_until("get_mcu_bootcfg", "Raspberry Pi Pico")
        if self.mcu_boot_mode_set(mode=mode, response=resp):
            return self.validate_usb(mode=mode)
        time.sleep(1)

        resp = self._ser.async_send_read_message_until(f"mcu_{mode}", "Raspberry Pi Pico")
        if "response: ok" not in str(resp):
            print(f"Failed to set MCU boot mode\n{resp})")
            return False
        time.sleep(2)

        return True
       
    def reset_hard_mcu(self):
        resp = self._ser.async_send_read_message_until(f"reset_mcu", "Raspberry Pi Pico")
        if "response: ok" not in str(resp):
            print(f"Failed to hard reset  MCU\n{resp})")
            return False
        time.sleep(2)

        return True
    
    def check_usb_app_zephyr(self):
        if self.validate_usb("app") is False:
            print(f"USB ZEPHYR not found: Reset to APP mode")
            if self.set_mcu_boot_mode("app") is False:
                print(f"Failed to set MCU app mode")
                return False
            if self.reset_hard_mcu() is False:
                print(f"Failed to hard reset MCU")
                return False
            if self.validate_usb("app") is False:
                print(f"Failed to validate USB ZEPHYR")
                return False
            
        return True