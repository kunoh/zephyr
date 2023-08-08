#!/usr/bin/python3
import subprocess
import logging
import sys
import os
import time
from serial import *
from threading import Thread
from threading import Event

# Parameters
dev_str = '/dev/serial/by-id/usb-SEGGER_J-Link_001067297035-if00'
iterations = 2
period_reset = 4


last_received = ''

def run_cmd(cmd):
    logging.info("Running: %s", cmd)
    try:
        return subprocess.run(cmd, check=True, shell=True, executable="/bin/bash")
    except:
        logging.error("Error running cmd %s. Try clean build or build.py update", cmd)

def receiving(event, ser, file):
    global last_received
    buffer_string = ''
    while True:
        if event.is_set():
            break

        buffer_string = ser.read(ser.inWaiting()).decode()
        if buffer_string:
            file.write(buffer_string)
            buffer_string = ''

def main():
    logging.basicConfig(level=logging.INFO)
    script_dir = os.path.dirname(__file__)
    serial_log_file = os.path.join(script_dir, "tmp/reset_tmp.txt")
    results_file = os.path.join(script_dir, "tmp/search_res.txt")
    event = Event()
    ser = Serial(dev_str, 115200)
    print("Writing serial receive log to: ", serial_log_file)
    file = open(serial_log_file, 'w')
    i = 0
    
    # Start receiver thread
    thread = Thread(target=receiving, args=(event, ser, file))
    thread.start()

    # Reset loop
    while i < iterations:
        print("Iteration: ", i)
        run_cmd(f"sudo ~/go/bin/mcumgr --conntype serial --connstring '/dev/serial/by-id/usb-ZEPHYR_TM5_IO_DEVICE_5001007C314B41D7-if00,baud=115200' reset")
        print()
        i += 1
        time.sleep(period_reset)

    event.set()
    thread.join()
    file.close()
    ser.close()

    # Find fault in log
    words = ["FAULT"]
    match_num = 0
    match = False
    print("Searching ", serial_log_file)
    file_res = open(results_file, 'w')
    with open(serial_log_file, 'r') as fp:
        for word in words:
            fp.seek(0)
            for line_num, line in enumerate(fp):
                if word in line:
                    match = True
                    match_num += 1
                    file_res.write("\nString match found.\n")
                    file_res.write(f"Line number: {line_num}\n")
                    file_res.write(f"Line: {line}\n")
        
    if not match:
        print("Did not find a match.")
        file_res.write("Did not find a match.")
    else:
        print(f"Found {match_num} matches.")
        print(f"Results written to: {results_file}")

    file_res.close()


if __name__ == '__main__':
    main()