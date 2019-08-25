#!/usr/bin/env python3
"""Colorize your output using RegEx"""

import argparse
import os
import select
import sys

# Need to backward-support legacy config and want support for variable-length
# lookbehinds.
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
                        default='.testrc')  # TODO: '~/.chromatermrc')
    parser.add_argument('--demo',
                        action='store_true',
                        help='demo the available custom color-codes')

    return parser.parse_args()


def color_demo():
    """Print the 256-color demo."""
    # TODO
    print('Demo')


def eprint(*args, **kwargs):
    """Error print."""
    print(sys.argv[0] + ':', *args, file=sys.stderr, **kwargs)


def get_color_code(color):
    """Return the ANSI code to be used when highlighting for the `color`."""
    code = '\033[33m'

    return code


def get_highlight_repl_func(config, color):
    """Return the replace function for a `color` to be used in `re.sub`."""
    color_code = get_color_code(color)

    def func(match):
        return color_code + match.group(0) + config['reset_string']

    return func


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
    """Return a dict of the parsed `rule` if parsed successfully. If not, a
    string with the error message is returned."""
    # pylint: disable=too-many-return-statements

    regex_str = rule.get('regex')
    if not regex_str:
        return 'regex not found'

    if not isinstance(regex_str, str) and not isinstance(regex_str, int):
        return 'regex not string or integer'

    color = rule.get('color')
    if not color:
        return 'color not found'

    if not isinstance(color, str) and not isinstance(color, dict):
        return 'color not string or dictionary'

    if isinstance(color, dict):  # Complex action
        for sub_action in color:
            if not isinstance(sub_action, int):
                return 'sub-action {} not integer'.format(sub_action)

            if not isinstance(color[sub_action], str):
                return 'sub-action {} value not string '.format(sub_action)

    return {
        'regex': regex_str,
        'color': color,
        'repl_func': get_highlight_repl_func(config, color),
    }


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


def main():
    """Main entry point."""
    args = args_init()
    buffer = ''

    if args.demo:
        return color_demo()

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
