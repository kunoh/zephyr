import usb.core
import usb.util
from . import log

class UsbHid:
    def __init__(self, vid=0x246D, pid=0x1050, interface=1) -> None:
        self.interface = interface
        self.dev = usb.core.find(idVendor=vid, idProduct=pid)
        self.endpoint = self.dev[0][(0,0)][0]

        #self._show_config()

        if self.dev.is_kernel_driver_active(interface):
            log.inf( 'detaching kernel driver')
            self.dev.detach_kernel_driver(interface)
            usb.util.claim_interface(self.dev, interface)

    def _show_config(self):
        for cfg in self.dev:
            log.inf(str(cfg.bConfigurationValue) + '\n')
            for intf in cfg:
                log.inf('\t' + \
                                str(intf.bInterfaceNumber) + \
                                ',' + \
                                str(intf.bAlternateSetting) + \
                                '\n')
                for ep in intf:
                    log.inf('\t\t' + \
                                    str(ep.bEndpointAddress) + \
                                    '\n')

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
        return bytes(self.dev.read(self.endpoint.bEndpointAddress + 1, self.endpoint.wMaxPacketSize, 1000*60))
