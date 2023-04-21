import serial

class Serial():
    def port_is_available(port:str="/dev/mnn"):
        ser = serial.Serial()
        ser.braudrate = 115200
        ser.port = str(port)
        try:
            ser.open()
        except Exception as e:
            print(f"{e}")
        p_is_available = False
        if ser.isOpen():
            p_is_available = True
            ser.close()
        return p_is_available
        