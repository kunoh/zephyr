import argparse
import serial
import signal
import os, sys, time

TEST_TIME_OUT = 12000 # 20 minutes
g_cancel_all = False

def handle_signal_ctrl_c(signum, frame):
    global g_cancel_all
    g_cancel_all = True
    print(f"Ctrl-C received ({signum})")

def scan_port(usb_port:str="", destination_path:str="", destination_file:str=""):
    global g_cancel_all
    print(f"Collect prints from {usb_port}")
    file = os.path.join(destination_path, destination_file)
    print(f"To file {file}")

    try:
        ser_hdl = serial.Serial(
            port=usb_port,
            baudrate=115200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS
        )
    except Exception as e:
        print(f"Failed to get serial com port: {e}")
        return 1

    if not ser_hdl.is_open:
        print(f"Failed to verify serial open")
        return 1

    try:
        fd = open(file, 'w')
    except Exception as e:
        print(f"Failed to open destination file: {file}")
        return 1

    buff = ""
    last_buff = buff
    time_start = time.time()
    while TEST_TIME_OUT > (time.time() - time_start):
        ser_hdl.timeout = 1.0
        buff = str(ser_hdl.readline().decode())
        if str(buff) != str(last_buff):
            fd.write(buff)
            last_buff = str(buff)
        if g_cancel_all:
            print("Prints collection completed")
            fd.close()
            if ser_hdl.is_open:
                ser_hdl.close()
            break

    return 0

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="print_collector",
        description="Collect prints from serial usb"
    )

    parser.add_argument('-s', '--serial-port', required=True)
    parser.add_argument('-p', '--log-file-path', required=True)
    parser.add_argument('-f', '--log-file-name', required=True)
    args = parser.parse_args()

    if not os.path.isdir(args.log_file_path):
        print(f"Dir '{args.log_file_path}' not found")
        sys.exit(1)

    pid = str(os.getpid())
    with open(os.path.join(args.log_file_path, 'tmp_pid'), 'w') as fd:
        fd.write(pid)

    signal.signal(signal.SIGINT, handle_signal_ctrl_c)

    sys.exit(scan_port(args.serial_port, args.log_file_path, args.log_file_name))