import usb.core
import usb.util
from . import log
import re

class UsbHid:
    def __init__(self, vid=0x246D, pid=0x1050, interface=3) -> None:
        self.interface = interface
        self.dev = usb.core.find(idVendor=vid, idProduct=pid)
        self.dev_hid = self._get_hid_interface()
        self._interface = self._get_hid_bInterfaceNumber()
        if not self._interface:
            self._interface = interface

        #self._show_hid_interface()

        if self.dev.is_kernel_driver_active(self._interface):
            log.inf( 'detaching kernel driver')
            self.dev.detach_kernel_driver(self._interface)
            usb.util.claim_interface(self.dev, self._interface)

    def _show_hid_interface(self):
        for k,v in self.dev_hid.items():
            log.inf(f"{k} : {v}")
           
    def _get_hid_interface(self):
        config_list = {}
        data = []
        config_count = -1
        interf_count = -1
        hid_config_index = -1
        hid_interface_index = -1
        new_data = False
        for cfg in self.dev:
            for line in str(cfg).splitlines():
                if "CONFIGURATION" in line:
                    config_count += 1
                    interf_count = -1
                    config_list[f'CONFIGURATION_{config_count}'] = {}
                elif "INTERFACE" in line and config_count != -1:
                    if new_data:
                        config_list[f'CONFIGURATION_{config_count}'][f'INTEFACE_{interf_count}'] = data
                    interf_count += 1
                    data = []
                    new_data = False
                elif interf_count != -1:
                    new_data = True
                    data.append(line)
                    if "Human Interface Device" in line:
                        hid_config_index = config_count
                        hid_interface_index = interf_count

            if new_data:
                # Don't forget the last data
                config_list[f'CONFIGURATION_{config_count}'][f'INTEFACE_{interf_count}'] = data
                new_data = False
            ret_data = {}

            for i in range(len(config_list[f'CONFIGURATION_{hid_config_index}'][f'INTEFACE_{hid_interface_index}'])):
                kv_pair = config_list[f'CONFIGURATION_{hid_config_index}'][f'INTEFACE_{hid_interface_index}'][i]
                kv_pair = str(kv_pair).strip().split(":")
                key = kv_pair[0].strip()
                val = kv_pair[1].strip()
                last_key = ""
                if key in ret_data:
                    # Handle multiple entries: bLength, bDescriptorType, bEndpointAddress, bmAttributes, Transfer Type, Synch Type, etc  
                    ret_data[f"{key}_2"] = val if len(kv_pair) == 2 else log.inf(f"No value for key {kv_pair}")
                else:
                    ret_data[key] = val if len(kv_pair) == 2 else log.inf(f"No value for key {kv_pair}")

            return ret_data
        
    def _get_hid_bEndpointAddress(self):
        bEndpointAddress = str(self.dev_hid['bEndpointAddress'])
        bEndpointAddress = re.match(r"0x\d+", bEndpointAddress).group(0)
        #print(bEndpointAddress)
        return int(bEndpointAddress, 16)

    def _get_hid_bInterfaceNumber(self):
        bInterfaceNumber = str(self.dev_hid['bInterfaceNumber'])
        bInterfaceNumber = re.match(r"0x\d+", bInterfaceNumber).group(0)
        #print(bInterfaceNumber)
        return int(bInterfaceNumber, 16)
    
    def _get_hid_wMaxPacketSize(self):
        wMaxPacketSize = str(self.dev_hid['wMaxPacketSize'])
        wMaxPacketSize = re.match(r"0x\d+", wMaxPacketSize).group(0)
        #print(wMaxPacketSize)
        return int(wMaxPacketSize, 16)
    
    def _hid_set_report(self, report):
        """ Implements HID SetReport via USB control transfer """
        self.dev.ctrl_transfer(
            0x21,  # REQUEST_TYPE_CLASS | RECIPIENT_INTERFACE | ENDPOINT_OUT
            9,     # SET_REPORT
            0x200, # "Vendor" Descriptor Type + 0 Descriptor Index
            self.interface,     # USB interface № 0
            report # the HID payload as a byte array -- e.g. from struct.pack()
        )

    def _hid_get_report(self):
        """ Implements HID GetReport via USB control transfer """
        return self.dev.ctrl_transfer(
            0xA1,  # REQUEST_TYPE_CLASS | RECIPIENT_INTERFACE | ENDPOINT_IN
            1,     # GET_REPORT
            0x200, # "Vendor" Descriptor Type + 0 Descriptor Index
            self.interface,     # USB interface № 0
            64     # max reply size
        )

    def _hid_get_descriptor(self):
        """ Implements HID GetDescriptor via USB control transfer """
        return self.dev.ctrl_transfer(
            0x81,  # REQUEST_TYPE_CLASS | RECIPIENT_INTERFACE | ENDPOINT_IN
            6,     # GET_DESCRIPTOR
            0x2200, # "Vendor" Descriptor Type + 0 Descriptor Index
            self.interface,     # USB interface № 0
            64     # max reply size
        )

    def send(self, data):
        self._hid_set_report(data)

    def read(self):
        return bytes(self.dev.read(self._get_hid_bEndpointAddress(), self._get_hid_wMaxPacketSize(), 1000*60))

