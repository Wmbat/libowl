"""
file: libowl/detail/x11/generate_keysym_to_code_point_table.py
author: wmbat-dev@protonmail.com
brief: Script used to fetch all key symbols to unicode code points supported by the X server and 
create a lookup table mapping each key symbol to it's code point equivalent
copyright: Copyright (C) 2022 wmbat.
"""

import os
import io 
import re
import subprocess

from typing import List

output_dir = os.path.dirname(os.path.relpath(__file__))
output_filename = 'keysym_to_code_point_table.hpp'
output_filepath = output_dir + '/' + output_filename

keysym_to_UCS_pattern_01 = re.compile(
        r'^\#define XK_([a-zA-z_0-9]+)\s+0x([0-9a-f]+)\s*\/\* U\+([0-9A-F]{4,6})(.*)\*\/\s*$')
keysym_to_UCS_pattern_02 = re.compile(
        r'^\#define XK_([a-zA-Z_0-9]+)\s+0x([0-9a-f]+)\s*\/\*\(U\+([0-9A-F]{4,6}) (.*)\)\*\/\s*$')

def file_description():
    dir = '/' + output_dir;
    dir = os.path.join(*(dir.split(os.path.sep)[2:]))
    filepath = dir + '/' + output_filename

    str = '/**\n'
    str = str + f' * @file {filepath}\n'
    str = str +  ' * @author wmbat-dev@protonmail.com\n'
    str = str +  ' * brief Table of X server keysym to their unicode code point equivalent'
    str = str +  ' * @copyright Copyright (C) 2022 wmbat\n */'

    return str


def header_guard():
    dir = '/' + output_dir;
    dir = os.path.join(*(dir.split(os.path.sep)[2:]))
    filepath = dir + '/' + output_filename
    return filepath.replace('/', '_').replace('.', '_').upper() + "_"

def extract_key_value_pair(line: str):
    key = re.search(r'0x([0-9a-f]+)*', line)
    val = re.search(r'U\+([0-9A-F]{4,6})', line)

    return key.group(), val.group()


def parse_line(line: str):
    if keysym_to_UCS_pattern_01.match(line) or keysym_to_UCS_pattern_02.match(line):
        key, val = extract_key_value_pair(line)

        return True, key, re.sub(r'U\+', '0x', val)
    else:
        return False, '', ''



def parse_x11_keysim_data(reader: io.TextIOWrapper) -> List:
    lines = reader.readlines()

    key_val_pairs = []

    for line in lines:
        is_parsed, key, val = parse_line(line)

        if (is_parsed):
            key_val_pairs.append((key, val))

    return key_val_pairs


def main():
    table_data = []
    with open('/usr/include/X11/keysymdef.h', 'r') as reader: 
        table_data = parse_x11_keysim_data(reader)

    with open(output_filepath, 'w') as writer:
        data = ''
        for key, val in table_data:
            str = 'keysym_codepoint_pair{'
            str = str + f'.keysym = keysym_t({key}u),'
            str = str + f'.code_point = owl::detail::code_point_t({val}u)}},\n'
            data = data + str

        header_guard_name = header_guard()

        writer.write(f'{file_description()}\n\n')

        writer.write('#ifndef {}\n'.format(header_guard_name))
        writer.write('#define {}\n\n'.format(header_guard_name))

        writer.write('#include <libowl/types.hpp>\n\n')

        writer.write('#include <array>\n\n')

        writer.write('namespace owl::inline v0\n{')
        writer.write('namespace x11\n{')

        writer.write('struct keysym_codepoint_pair{keysym_t keysym; owl::detail::code_point_t code_point;};')
 
        writer.write('static constexpr std::array<keysym_codepoint_pair, {}> keysym_to_code_point_table = '
            .format(len(table_data)))
        writer.write('{{\n{}\n}};'.format(data[:-2] + '\n'))

        writer.write('}\n}\n\n')

        writer.write('\n#endif // {}'.format(header_guard_name))

    format_output = subprocess.run(
            ['clang-format', output_filepath], 
            encoding='utf-8',
            stdout=subprocess.PIPE)

    with open(output_filepath, 'w') as writer:
        writer.seek(0)
        writer.write(format_output.stdout)
        writer.truncate()

if __name__ == '__main__':
    main()
