#!/usr/bin/env python3
"""Colorize your output using RegEx"""

import argparse
import os
import select
import sys

import regex as re
import yaml

# Maximum chuck size per read
READ_SIZE = 65536  # 64 KiB

# CT cannot determine if it is processing input faster than the piping process
# is outputting or if the input has finished. To work around this, CT will wait
# a bit prior to assuming there's no more data in the buffer. There's no impact
# on performace as the wait returns if stdin becomes ready to be read from.
WAIT_FOR_NEW_LINE = 0.0005


def args_init():
    """Initialzes arguments and returns the output of `parse_args`."""
    parser = argparse.ArgumentParser(description='Colorize your output using'
                                     'RegEx.')

    parser.add_argument('--config',
                        metavar='FILE',
                        type=str,
                        help='location of config file (default: %(default)s)',
                        default='.chromatermrc')  # TODO:'$HOME/.chromatermrc')

    return parser.parse_args()


def eprint(*args, **kwargs):
    """Error print."""
    print(sys.argv[0] + ':', *args, file=sys.stderr, **kwargs)


def get_color_code(color):
    """Return the ANSI code to be used when highlighting with `color` or None if
    the `color` is invalid. The `color` is a string in the format of b#abcdef
    for background or f#abcdef for foreground. Can be multiple colors if
    separated by a space."""
    if not re.match(r'^((b|f)#([0-9a-fA-F]{6})(\s|$))+$', color):
        return None

    code = ''

    for match in re.findall(r'(b|f)#([0-9a-fA-F]{6})', color):
        target = '\033[38;5;' if match[0] == 'f' else '\033[48;5;'
        rgb = (int(match[1][i:i + 2], 16) for i in [0, 2, 4])
        color_id = rgb_to_8bit(*rgb)
        code += target + str(color_id) + 'm'

    return code or None


def highlight(rules, line):
    """According to the `rules`, return a highlighted 'line'."""
    for rule in rules:
        line = re.sub(rule['regex'], rule['repl_func'], line)

    return line


def parse_config(data):
    """Parse `data` (a YAML string), returning a dictionary of the config."""
    config = {'rules': [], 'reset_string': '\033[0m'}

    try:  # Load the YAML configuration file
        load = yaml.safe_load(data)
    except yaml.YAMLError as exception:
        eprint('Parse error:', exception)
        return config

    # Parse the rules
    rules = load.get('rules', [])
    rules = rules if isinstance(rules, list) else []

    for rule in rules:
        parsed_rule = parse_rule(rule, config)
        if isinstance(parsed_rule, dict):
            config['rules'].append(parsed_rule)
        else:
            eprint('Rule error on {}: {}'.format(rule, parsed_rule))

    return config


def parse_rule(rule, config):
    """Return a dict from `get_highlight_repl_func` if parsed successfully. If
    not, a string with the error message is returned."""
    # pylint: disable=too-many-return-statements

    regex = rule.get('regex')
    if regex is None:
        return 'regex not found'

    if not isinstance(regex, str) and not isinstance(regex, int):
        return 'regex not a string or integer'

    color = rule.get('color')
    if color is None:
        return 'color not found'

    if not isinstance(color, str):
        return 'color not a string'

    color_code = get_color_code(color)
    if not color_code:
        return 'color not in the correct format'

    group = rule.get('group', 0)
    if group is None:
        return 'group not found'

    if not isinstance(group, int):
        return 'group not an integer'

    group_count = re.compile(regex).groups
    if group > group_count:
        return 'group ID over the number of groups in the regex'

    def func(match):
        return color_code + match.group(group) + config['reset_string']

    return {'regex': regex, 'repl_func': func}


def process_buffer(config, buffer, more):
    """Process the `buffer` using the `config`, returning any left-over data.
    If there's `more` data coming up, only process full lines (ending with new
    line). Otherwise, process all of the buffer."""
    lines = buffer.splitlines(True)  # Keep line-splits

    if not lines:
        return ''

    for line in lines[:-1]:  # Process all lines except for the last
        print(highlight(config['rules'], line), end='')

    # More data to come; hold off on printing the last line
    if read_ready(WAIT_FOR_NEW_LINE) and more:
        return lines[-1]  # Return last line as the left-over data

    # No more data; print last line and flush as it doesn't have a new line
    print(highlight(config['rules'], lines[-1]), end='', flush=True)

    return ''  # All of the buffer was processed; return an empty buffer


def read_ready(timeout=None):
    """Return True if sys.stdin has data or is closed. If `timeout` is None,
    this function will block until the True condition is met. If `timeout` is
    specified, the function will return False if expired or True as soon as the
    condition is met."""
    return sys.stdin in select.select([sys.stdin], [], [], timeout)[0]


def rgb_to_8bit(_r, _g, _b):
    """Downscale from 24-bit RGB to 8-bit ANSI."""
    def downscale(value):
        return int(value / 256 * 6)

    return 16 + (36 * downscale(_r)) + (6 * downscale(_g)) + downscale(_b)


def main():
    """Main entry point."""
    args = args_init()
    buffer = ''

    with open(args.config, 'r') as file:
        config = parse_config(file)

    while read_ready():
        data = os.read(sys.stdin.fileno(), READ_SIZE)
        buffer += data.decode()

        if not buffer:  # Entire buffer was processed empty and fd is closed
            break

        # Process the buffer, updating it with any left-over data
        buffer = process_buffer(config, buffer, bool(data))


if __name__ == '__main__':
    sys.exit(main())
