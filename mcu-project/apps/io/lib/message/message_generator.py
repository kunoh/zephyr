import re
import os
import sys
from pathlib import Path
from jinja2 import Environment, FileSystemLoader

src_proto = sys.argv[1]
out_dir = sys.argv[2]
templ_dir = sys.argv[3]

type_proto_to_cpp = {
    'bool': 'bool',
    'int32': 'int',
    'float': 'float',
    'double': 'double',
    'string': 'char*',
    'bytes': 'uint8_t*'
}
file_loader = FileSystemLoader(templ_dir)
env = Environment(loader=file_loader)

basename = Path(src_proto).stem

# Parse messages from .proto file
with open(src_proto) as f:
    proto_data = f.read()

data = {
    'basename': basename,
    'functions': []
}

for item in proto_data.split("\n"):
    if "}" in item:
        if bool(function):
            data['functions'].append(function)
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

    for proto_type, cpp_type in type_proto_to_cpp.items():
        if proto_type in item:
            variable_name = re.sub(rf'{proto_type} (\S+).*', r'\1', item).strip()
            function['variables'].append(
                {
                    'name': variable_name,
                    'type': cpp_type
                })
            break

if not os.path.exists(out_dir):
    os.makedirs(out_dir)

def generate(template, out_file):
    template = env.get_template(template)
    output = template.render(data)

    with open(os.path.join(out_dir, out_file), 'w') as f:
        f.write(output)

# Generate message_handler.cpp
generate('x_message_handler_cpp.template', basename + '_' + 'message_handler.cpp')

# Generate message_handler.h
generate('x_message_handler_h.template', basename + '_' + 'message_handler.h')

# Generate message_encoder.cpp
generate('x_message_encoder_cpp.template', basename + '_' + 'message_encoder.cpp')

# Generate message_encoder.h
generate('x_message_encoder_h.template', basename + '_' + 'message_encoder.h')