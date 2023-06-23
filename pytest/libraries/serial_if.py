import serial, shutil
import os, sys, time
import asyncio, serial_asyncio

sys.path.append(f"{os.path.curdir}/scripts")        
from libs import log, util, usb_hid

RECV_MSG_LEN = 1024

class SerialMcu():
    def __init__(self, port:str="/dev/mnn") -> None:
        self._ser = serial.Serial()
        self._ser.baudrate = 115200
        self._ser.port = str(port)
        pass

    def port_is_available(self):
        try:
            self._ser.open()
            if self._ser.is_open:
                self._ser.close()
                return True
        except Exception as e:
            print(f"{e}")
            return False
    
    def send_recv_message(self, msg:str="", expected:str="\n", rw:str="rw", timeout=30):
        print(f"Send and Recv from {self._ser.port}")
        self._ser.write_timeout = timeout
        self._ser.timeout = timeout # read timeout
        try:
            self._ser.open()
            
            if not self._ser.is_open:
                print(f"Failed to open {self._ser.port}")
                return None
            data = None

            if rw == "rw" or rw == "w":
                print(f"Send msg: {msg}")
                self._ser.write(str(msg).encode())
                time.sleep(1)
            if rw == "rw" or rw == "r":
                #print(f"Receiving")
                data = ""
                time_start = time.time()
                while expected not in data and timeout > (time.time() - time_start):
                    data += str(self._ser.read_until(expected.encode(), RECV_MSG_LEN).decode())
                    #print(f"{data}")
            
            self._ser.close()
            
            return data
        except Exception as e:
            print(f"{e}")
            return None
        
class SerialMcuAsync:
    def __init__(self, port):
        self._baudrate = 115200
        self._port = port

    def _set_baudrate(self, baudrate):
        self._baudrate = baudrate

    async def _write_and_read(self, loop, msg, expected, port):
        reader, writer = await serial_asyncio.open_serial_connection(url=port, baudrate=self._baudrate)
        message = msg.encode('utf-8')+b'\r'
        sent = self._send(writer, message)
        received = asyncio.gather(self._recv(reader, expected.encode('utf-8')))
        await asyncio.wait([sent, received]) # TODO: Fix DeprecationWarning: The explicit passing of coroutine objects to asyncio.wait() is deprecated since Python 3.8
        return received.result()[0].decode()

    async def _send(self, w, msg):
        await w.write(msg)
        return True

    async def _recv(self, r, expect):
        msg = await r.readuntil(expect)
        return msg

    def async_send_read_message_until(self, msg, expected):
        print(f"Execute serial command [{msg}]")
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop=loop)
        for retry in range(0, 3):
            try:
                loop.call_later(3, loop.stop)
                received = loop.run_until_complete(self._write_and_read(loop, msg, expected, self._port))
            except Exception:
                received = 'Async serial r/w failed!'
            if not received or received == 'Async serial r/w failed!':
                print(f"Retry {retry} on port {self._port}")
                continue
            break
        loop.close()
        return received.replace("\0", "")
    
class MessageIF():
    def __init__(self, interface, com_protocol, root_dir) -> None:
        self._libs_root_dir = os.path.join(root_dir, "scripts")
        if not os.path.exists(self._libs_root_dir):
            print("Missing libs root directory: Exit Failure")
            exit(1)
        if not self._generate_dependencies(interface):
            print("Failed to generate libs dependencies: Exit Failure")
            exit(1)
        self._com_proto_lib = usb_hid.UsbHid(interface=3) if com_protocol == "usb" else ""

    def _generate_dependencies(self, interf):
        print(f"Remove old files")
        if os.path.exists(os.path.join(self._libs_root_dir, 'libs/proto')):
            shutil.rmtree(os.path.join(self._libs_root_dir, 'libs/proto'))
        if os.path.isfile(os.path.join(self._libs_root_dir, 'libs/io_interface.py')):
            os.remove(os.path.join(self._libs_root_dir, 'libs/io_interface.py'))
        print(f"Create {interf} lib and proto files to {self._libs_root_dir}")
        res = util.run_cmd(f"python3 {os.path.join(self._libs_root_dir, 'libs/generate_files.py')} -t {interf}")
        from libs import io_interface
        self._iface_lib = io_interface.Io() if interf == "io" else ""
        return True

    def get_messages(self):
        print("Get IO Interface messages list")
        try:
            return self._iface_lib.get_messages()
        except Exception as e:
            print(f"Invalide Interface Library {self._iface_lib}")
            return None

    def send_recv_message(self, msg_type=None, msg=None):
        print("Usb send and recv message")
        input = []
        input.append(msg_type) if msg_type else print("No message type")
        input.append(msg) if msg else print("No message")
        print(f"Test Msg: {input}")

        encoded_message = self._iface_lib.handle_cli_input(input)
        print(f"Encoded Send Msg: {encoded_message}")
        if encoded_message == None:
            return None
        
        self._com_proto_lib.send(encoded_message)
        resp = self._com_proto_lib.read()
        print(f"Encoded Recv Msg: {resp}")
        decoded_message = self._iface_lib.handle_response(resp, True)

        return decoded_message
