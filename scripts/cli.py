#!/usr/bin/python3

import os
import argparse
import subprocess
from libs import log, util, usb_hid

# Interfaces will be initialized after code generation and import
# It is used as a forward declaration, to tell available targets for argparse
INTERFACES = \
{
    'io': None
}

COMMUNICATION_PROTOCOLS = \
{
    'usb': lambda: usb_hid.UsbHid()
}

class Cli:
    def __init__(self, interface, communication_protocol) -> None:
        self.iface = interface
        self.cp = communication_protocol
        self._messages = self.iface.get_messages()
        self._menu = {}

    def show_help(self):
        print('Write "help" to get a list of commands')
        print('Write "exit" to exit cli tool')
        print('')
        print('Choose the message by using the number plus arguments:')
        print('')
        self.show_menu()

    def show_menu(self):
        counter = 0
        for area_key in self._messages.keys():
            print(area_key[:1].upper() + area_key[1:])
            for function in self._messages[area_key]['functions']:
                if 'Response' in function["name"]:
                    continue
                self._menu[str(counter)] = function['name']
                print(f'{counter}. {function["name"]}', end=" ")
                for var in function['variables']:
                    print(f'<{var["name"]}>', end=" ")
                print('')
                counter += 1

    def validate_input(self, input):
        if input not in self._menu.keys():
            log.wrn(f'Key: "{input}" is not valid')
            return False
        return True

    def convert_user_input_to_message(self, input):
        return self._menu[input[0]]

    def run(self):

        self.show_help()

        while(1):

            # GET INPUT KEY
            print('>', end=" ")
            input_key = input().split(' ')
            if input_key[0] == 'help':
                self.show_help()
                continue
            elif input_key[0] == 'exit':
                break
            elif not self.validate_input(input_key[0]):
                self.show_help()
                continue

            input_key[0] = self.convert_user_input_to_message(input_key)
            # ENCODE MESSAGE
            encoded_message = self.iface.handle_cli_input(input_key)
            if encoded_message == None:
                continue

            # SEND OVER USB / UART
            self.cp.send(encoded_message)

            # GET RESPONSE
            response = self.cp.read()

            print('')

            # HANDLE RESPONSE
            self.iface.handle_response(response)

            print('')
            print('------------------------')
            print('')


def main():
    global INTERFACES
    parser = argparse.ArgumentParser()
    parser.add_argument("-t", "--target", required=True, type=str, choices=INTERFACES.keys())
    parser.add_argument("-cp", "--communication_protocol", required=True, type=str, choices=COMMUNICATION_PROTOCOLS.keys())

    args = parser.parse_args()

    res = util.run_cmd(f"python3 {os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/generate_files.py')} -t {args.target}")

    try:
        from libs import io_interface
    except Exception:
        pass
    try:
        from libs import ble_interface
    except Exception:
        pass

    INTERFACES['io'] = lambda: io_interface.Io()

    interface = INTERFACES[args.target]()
    communication_protocol = COMMUNICATION_PROTOCOLS[args.communication_protocol]()

    cli = Cli(interface, communication_protocol)

    cli.run()

if __name__ == "__main__":
    main()