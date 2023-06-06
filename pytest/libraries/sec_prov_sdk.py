from typing import Optional, Union
import os, subprocess

class SecProvSDK():
    def __init__(self, tool_path) -> None:
        self._flashloader_tool_path = tool_path
        self._tool_sdphost = os.path.join(self._flashloader_tool_path, "sdphost/linux/amd64/sdphost")
        self._tool_blhost = os.path.join(self._flashloader_tool_path, "blhost/linux/amd64/blhost")
    
    def write_ivt_flashloader_to_internal_ram(self, hid="0x1fc9,0x0135"):
        '''
        flashloader_tool_path = "/home/tm/tools/Flashloader_RT106x_1.0_GA/Tools"
            sudo $(flashloader_tool_path)/sdphost/linux/amd64/sdphost -u 0x1fc9,0x0135 -t 5000 -- write-file 0x20000000 "$(flashloader_tool_path)/mfgtools-rel/Profiles/MXRT106X/OS Firmware/ivt_flashloader.bin"
            Preparing to send 90529 (0x161a1) bytes to the target.
            (1/1)0%Status (HAB mode) = 1450735702 (0x56787856) HAB disabled.
            Reponse Status = 2290649224 (0x88888888) Write File complete.
        '''
        cmd = f'sudo {self._tool_sdphost} -u 0x1fc9,0x0135 -t 5000 -- write-file 0x20000000 "{self._flashloader_tool_path}/mfgtools-rel/Profiles/MXRT106X/OS Firmware/ivt_flashloader.bin"'
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Write File complete" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def boot_ivt_flashloader_image(self, hid="0x1fc9,0x0135"):
        '''
        sudo $(flashloader_tool_path)/Tools/sdphost/linux/amd64/sdphost -u 0x1fc9,0x0135 -t 5000 -- jump-address 0x20000400
            Status (HAB mode) = 1450735702 (0x56787856) HAB disabled.
        '''
        cmd = f"sudo {self._tool_sdphost} -u 0x1fc9,0x0135 -t 5000 -- jump-address 0x20000400"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "HAB disabled" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def validate_blhost_connetcion(self, hid="0x15a2,0x0073"):
        '''
        sudo $(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -- get-property 1
            Inject command 'get-property'
            Response status = 0 (0x0) Success.
            Response word 1 = 1258422528 (0x4b020100)
            Current Version = K2.1.0

        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- get-property 1"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def write_optionblock_to_sram(self, hid="0x15a2,0x0073"):
        '''
        sudo $(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -- fill-memory 0x2000 0x04 0xc0000006
            Inject command 'fill-memory'
            Successful generic response to command 'fill-memory'
            Response status = 0 (0x0) Success.
        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- fill-memory 0x2000 0x04 0xc0000006"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def config_qspi_using_optionblock(self, hid="0x15a2,0x0073"):
        '''
        sudo $(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -- configure-memory 0x09 0x2000
            Inject command 'configure-memory'
            Successful generic response to command 'configure-memory'
            Response status = 0 (0x0) Success.
        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- configure-memory 0x09 0x2000"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def erase_all(self, hid="0x15a2,0x0073"):
        '''
        sudo $(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -t 20000 -- flash-erase-all 0x09
            Inject command 'flash-erase-all'
            Successful generic response to command 'flash-erase-all'
            Response status = 0 (0x0) Success.
        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -t 20000 -- flash-erase-all 0x09"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=30)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def flash_mcu_default_bootl(self, hid="0x15a2,0x0073", boot_file=""):
        '''
        TODO
        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- write-memory 0x60000000 {boot_file}"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def flash_mcu_default_app(self, hid="0x15a2,0x0073", firmware_file=""):
        '''
        sudo$(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -- write-memory 0x60020000 mcu_binaries/zephyr.signed-0.1.0-master.33.bin 
            Inject command 'write-memory'
            Preparing to send 262788 (0x40284) bytes to the target.
            Successful generic response to command 'write-memory'
            (1/1)100% Completed!
            Successful generic response to command 'write-memory'
            Response status = 0 (0x0) Success.
            Wrote 262788 of 262788 bytes.

        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- write-memory 0x60020000 {firmware_file}"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False

    def reset_mcu(self, hid="0x15a2,0x0073"):
        '''
        sudo $(flashloader_tool_path)/Tools/blhost/linux/amd64/blhost -u 0x15a2,0x0073 -- reset
            [sudo] password for tm: 
            Inject command 'reset'
            Successful generic response to command 'reset'
            Response status = 0 (0x0) Success.
        '''
        cmd = f"sudo {self._tool_blhost} -u 0x15a2,0x0073 -- reset"
        try:
            out = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, timeout=10)
            if "Response status = 0 (0x0) Success" in str(out):
                return True
        except Exception as e:
            print(f"{e}")
            return False
        return False