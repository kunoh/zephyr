#!/usr/bin/python3

import re
import os
import shutil
import argparse
from pathlib import Path
from jinja2 import Environment, FileSystemLoader

import util, log

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.join(SCRIPT_DIR, '../..')
APP_DIR = "mcu-project/apps"
APP_PROTO_DIR = "src/message_handlers/proto"
MODULES_DIR = "mcu-project/modules"
MODULES_PROTO_DIR = "message_handler"
PROTO_OUTPUT_DIR = os.path.join(SCRIPT_DIR, 'proto')


def compile_proto_files_from_path(proto_path):
    proto_files = []
    for root, dirs, files in os.walk(proto_path):
        for i, file in enumerate(files):
            if file.endswith(".proto"):
                util.run_cmd(f' \
                        protoc --proto_path={proto_path} --python_out={PROTO_OUTPUT_DIR} {os.path.join(root, file)} \
                        ')
                proto_files.append(f'{file[:-6]}_pb2')
    return proto_files

def generate_pb_files(mcu_target, modules_path_to_proto, app_path_to_proto):
    if os.path.exists(os.path.join(SCRIPT_DIR, 'proto')):
        shutil.rmtree(os.path.join(SCRIPT_DIR, 'proto'), )

    os.makedirs(os.path.join(SCRIPT_DIR, 'proto'))

    proto_files = []
    proto_files.extend(compile_proto_files_from_path(modules_path_to_proto))
    proto_files.extend(compile_proto_files_from_path(app_path_to_proto))

    log.inf('PROTO FILES SUCCESSFULLY GENERATED')

def get_proto_files(paths):
    proto_files = []
    for path in paths:
        for root, dirs, files in os.walk(path):
            for i, file in enumerate(files):
                if file.endswith(".proto"):
                    proto_files.append(os.path.join(root, file))
    return proto_files

def create_data_for_template(proto_files):
    proto_types = [
        'bool',
        'int32',
        'float',
        'double',
        'string',
        'bytes'
    ]
    data = {'messages': {}}

    for file in proto_files:

        if 'any' in file or 'outer' in file:
            continue

        basename = Path(file).stem

        data['messages'][basename] = \
        {
            'functions': []
        }

        with open(file) as f:
            proto_data = f.read()

        for item in proto_data.split("\n"):
            if "}" in item:
                if bool(function):
                    data['messages'][basename]['functions'].append(function)
                function = {}
                continue

            if "//" in item:
                continue

            if "message" in item and not "MessageOuter" in item:
                function = {
                        'name': item.split(' ')[1],
                        'variables': []
                    }
                continue

            for proto_type in proto_types:
                if proto_type in item:
                    variable_name = re.sub(rf'{proto_type} (\S+).*', r'\1', item).strip()
                    function['variables'].append(
                        {
                            'name': variable_name,
                            'type': proto_type
                        })
                    break

    return data

def generate(template, out_file, data):
    file_loader = FileSystemLoader(os.path.join(SCRIPT_DIR, 'templates'))
    env = Environment(loader=file_loader)

    def debug(text):
        print(text)
        return ''

    env.filters['debug']=debug

    template = env.get_template(template)
    output = template.render(data)

    with open(os.path.join(SCRIPT_DIR, 'proto', out_file), 'w') as f:
        f.write(output)

def generate_encoder_decoder(mcu_target, paths):
    proto_files = get_proto_files(paths)
    data = create_data_for_template(proto_files)
    data['target'] = mcu_target

    for template in ['encoder', 'decoder']:
        generate(f'{template}.template', f'{template}.py', data)
    generate('interface.template', f'../{mcu_target}_interface.py', data)

def main(mcu_target):
    modules_path_to_proto = os.path.join(ROOT_DIR, MODULES_DIR, MODULES_PROTO_DIR)
    app_path_to_proto = os.path.join(ROOT_DIR, APP_DIR, mcu_target, APP_PROTO_DIR)

    generate_pb_files(mcu_target, modules_path_to_proto, app_path_to_proto)

    generate_encoder_decoder(mcu_target, [modules_path_to_proto, app_path_to_proto])

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', '--target', choices=['io', 'ble'],
                        help='Build target')

    args = parser.parse_args()

    if not args.target:
        parser.error("Requires --target (-t)")

    main(args.target)
