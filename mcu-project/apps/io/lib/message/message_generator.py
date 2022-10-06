import re
import os
import sys
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

# Parse messages from .proto file
with open(src_proto) as f:
    proto_data = f.read()

data = {
    'functions' : []
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
        start_message = True
        function = {
                'name': item.split(' ')[1],
                'variables': []
            }
        continue
    
    for proto_type in type_proto_to_cpp.keys():
        if proto_type in item:
            variable_name = re.sub(rf'{proto_type} (\S+).*', r'\1', item).strip()
            function['variables'].append(
                {
                    'name': variable_name,
                    'type': type_proto_to_cpp[proto_type]
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
generate('message_handler_cpp.template', 'message_handler.cpp')

# Generate message_handler.h
generate('message_handler_h.template', 'message_handler.h')

# Generate message_encoder.cpp
generate('message_encoder_cpp.template', 'message_encoder.cpp')

# Generate message_encoder.h
generate('message_encoder_h.template', 'message_encoder.h')